#include "RHI/D3D12/ShaderCompiler.h"

#pragma comment(lib, "dxcompiler.lib")

namespace Lumina
{
    namespace
    {
        // リフレクションの (ComponentType, Mask) から DXGI フォーマットを決める。
        DXGI_FORMAT FormatFromParam(const D3D12_SIGNATURE_PARAMETER_DESC& p)
        {
            // Mask の立っているビット数 = 使用コンポーネント数(1..4)。
            u32 count = 0;
            for (u8 m = p.Mask; m; m >>= 1)
            {
                count += (m & 1u);
            }

            switch (p.ComponentType)
            {
            case D3D_REGISTER_COMPONENT_FLOAT32:
                return count == 1 ? DXGI_FORMAT_R32_FLOAT
                     : count == 2 ? DXGI_FORMAT_R32G32_FLOAT
                     : count == 3 ? DXGI_FORMAT_R32G32B32_FLOAT
                                  : DXGI_FORMAT_R32G32B32A32_FLOAT;
            case D3D_REGISTER_COMPONENT_UINT32:
                return count == 1 ? DXGI_FORMAT_R32_UINT
                     : count == 2 ? DXGI_FORMAT_R32G32_UINT
                     : count == 3 ? DXGI_FORMAT_R32G32B32_UINT
                                  : DXGI_FORMAT_R32G32B32A32_UINT;
            case D3D_REGISTER_COMPONENT_SINT32:
                return count == 1 ? DXGI_FORMAT_R32_SINT
                     : count == 2 ? DXGI_FORMAT_R32G32_SINT
                     : count == 3 ? DXGI_FORMAT_R32G32B32_SINT
                                  : DXGI_FORMAT_R32G32B32A32_SINT;
            default:
                return DXGI_FORMAT_UNKNOWN;
            }
        }
    }

    bool ShaderCompiler::Initialize()
    {
        LUMINA_CHECK_HR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils)),
                        "DxcCreateInstance(Utils) failed");
        LUMINA_CHECK_HR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler)),
                        "DxcCreateInstance(Compiler) failed");
        LUMINA_CHECK_HR(m_utils->CreateDefaultIncludeHandler(&m_includeHandler),
                        "CreateDefaultIncludeHandler failed");

        LogInfo("ShaderCompiler: initialized (DXC)");
        return true;
    }

    void ShaderCompiler::Shutdown()
    {
        m_includeHandler.Reset();
        m_compiler.Reset();
        m_utils.Reset();
    }

    bool ShaderCompiler::Compile(const std::wstring& path, const wchar_t* entry,
                                 const wchar_t* target, CompiledShader& out)
    {
        out = CompiledShader{};

        // ソース読み込み。
        ComPtr<IDxcBlobEncoding> source;
        if (m_utils->LoadFile(path.c_str(), nullptr, &source) < 0 || !source)
        {
            LogError("ShaderCompiler: failed to load %ls", path.c_str());
            return false;
        }
        const DxcBuffer srcBuffer{ source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_ACP };

        std::vector<LPCWSTR> args = {
            L"-E", entry,
            L"-T", target,
#ifdef _DEBUG
            L"-Zi", L"-Qembed_debug", L"-Od",
#else
            L"-O3",
#endif
        };

        ComPtr<IDxcResult> result;
        if (m_compiler->Compile(&srcBuffer, args.data(), static_cast<u32>(args.size()),
                                m_includeHandler.Get(), IID_PPV_ARGS(&result)) < 0)
        {
            LogError("ShaderCompiler: Compile() call failed");
            return false;
        }

        // コンパイルメッセージ(警告/エラー)を表示。
        ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetStringLength() > 0)
        {
            LogError("Shader messages (%ls):\n%s", path.c_str(), errors->GetStringPointer());
        }

        HRESULT status = S_OK;
        result->GetStatus(&status);
        if (status < 0)
        {
            return false;  // コンパイル失敗(呼び出し側で握る)
        }

        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&out.bytecode), nullptr);
        if (!out.bytecode)
        {
            LogError("ShaderCompiler: no object output");
            return false;
        }

        // リフレクション → 入力レイアウト(VSのみ実質的に使う)。
        ComPtr<IDxcBlob> reflection;
        result->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflection), nullptr);
        if (reflection)
        {
            BuildInputLayout(reflection.Get(), out);
        }
        return true;
    }

    bool ShaderCompiler::BuildInputLayout(IDxcBlob* reflectionBlob, CompiledShader& out)
    {
        const DxcBuffer reflBuffer{ reflectionBlob->GetBufferPointer(),
                                    reflectionBlob->GetBufferSize(), DXC_CP_ACP };

        ComPtr<ID3D12ShaderReflection> reflection;
        if (m_utils->CreateReflection(&reflBuffer, IID_PPV_ARGS(&reflection)) < 0)
        {
            return false;
        }

        D3D12_SHADER_DESC shaderDesc{};
        reflection->GetDesc(&shaderDesc);

        // reserve しておき、SemanticName 文字列の再配置(=ポインタ無効化)を防ぐ。
        out.semanticNames.reserve(shaderDesc.InputParameters);
        out.inputLayout.reserve(shaderDesc.InputParameters);

        for (u32 i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC param{};
            reflection->GetInputParameterDesc(i, &param);

            // SV_ 系(システム値)は頂点バッファに含まれないので除外。
            if (param.SystemValueType != D3D_NAME_UNDEFINED)
            {
                continue;
            }

            out.semanticNames.emplace_back(param.SemanticName);

            D3D12_INPUT_ELEMENT_DESC elem{};
            elem.SemanticName         = out.semanticNames.back().c_str();
            elem.SemanticIndex        = param.SemanticIndex;
            elem.Format               = FormatFromParam(param);
            elem.InputSlot            = 0;
            elem.AlignedByteOffset    = D3D12_APPEND_ALIGNED_ELEMENT;  // 自動詰め
            elem.InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elem.InstanceDataStepRate = 0;
            out.inputLayout.push_back(elem);
        }
        return true;
    }
}
