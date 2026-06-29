#pragma once

#include "RHI/D3D12/D3D12Common.h"

#include <dxcapi.h>
#include <d3d12shader.h>

#include <string>
#include <vector>

// Lumina Engine - RHI / D3D12 backend
// DXC で HLSL(SM6)を実行時コンパイルし、リフレクションで入力レイアウトを取り出す。
namespace Lumina
{
    // コンパイル結果。bytecode と、VSなら入力レイアウト(リフレクション由来)を持つ。
    struct CompiledShader
    {
        ComPtr<IDxcBlob>                      bytecode;
        std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;   // VS のみ意味を持つ
        std::vector<std::string>              semanticNames; // SemanticName の実体(寿命保持)

        bool Valid() const { return bytecode != nullptr; }

        D3D12_SHADER_BYTECODE ByteCode() const
        {
            return bytecode
                ? D3D12_SHADER_BYTECODE{ bytecode->GetBufferPointer(), bytecode->GetBufferSize() }
                : D3D12_SHADER_BYTECODE{ nullptr, 0 };
        }
    };

    class ShaderCompiler
    {
    public:
        bool Initialize();
        void Shutdown();

        // path=絶対パス, entry=L"VSMain", target=L"vs_6_0" など。
        // 失敗(コンパイルエラー含む)で false。エラー内容はログに出す。
        bool Compile(const std::wstring& path, const wchar_t* entry, const wchar_t* target,
                     CompiledShader& out);

    private:
        bool BuildInputLayout(IDxcBlob* reflectionBlob, CompiledShader& out);

        ComPtr<IDxcUtils>          m_utils;
        ComPtr<IDxcCompiler3>      m_compiler;
        ComPtr<IDxcIncludeHandler> m_includeHandler;
    };
}
