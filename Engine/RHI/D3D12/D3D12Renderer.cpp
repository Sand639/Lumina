#include "RHI/D3D12/D3D12Renderer.h"
#include "Core/Paths.h"

namespace Lumina
{

    D3D12Renderer::~D3D12Renderer()
    {
        Shutdown();
    }

    bool D3D12Renderer::Initialize(HWND hwnd, u32 width, u32 height)
    {
        m_width  = width;
        m_height = height;

        if (!m_device.Initialize())
        {
            return false;
        }
        if (!m_queue.Initialize(m_device.Device(), D3D12_COMMAND_LIST_TYPE_DIRECT))
        {
            return false;
        }
        if (!m_swapChain.Initialize(m_device.Factory(), m_queue.Get(), m_device.Device(),
                                    hwnd, width, height))
        {
            return false;
        }

        ID3D12Device* device = m_device.Device();

        // フレームごとにコマンドアロケータを用意(GPU使用中のものをResetしないため)。
        for (u32 i = 0; i < kFrameCount; ++i)
        {
            LUMINA_CHECK_HR(
                device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                               IID_PPV_ARGS(&m_allocators[i])),
                "CreateCommandAllocator failed");
        }

        // コマンドリストは1本を毎フレーム使い回す(Reset で付け替える)。
        LUMINA_CHECK_HR(
            device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                      m_allocators[0].Get(), nullptr,
                                      IID_PPV_ARGS(&m_commandList)),
            "CreateCommandList failed");
        // 生成直後は記録中(open)なので一旦閉じる。Render の先頭で Reset する。
        m_commandList->Close();

        if (!CreateTriangleResources(device))
        {
            return false;
        }

        if (!m_shaderCompiler.Initialize())
        {
            return false;
        }
        m_shaderPath = GetAssetsDir() + L"Shaders\\Triangle.hlsl";

        if (!CreatePipeline(device))
        {
            return false;
        }

        LogInfo("D3D12Renderer: initialized");
        return true;
    }

    bool D3D12Renderer::CreateTriangleResources(ID3D12Device* device)
    {
        if (!m_upload.Initialize(device, &m_queue))
        {
            return false;
        }

        const Vertex triangle[] = {
            { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },  // 上: 赤
            { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },  // 右下: 緑
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },  // 左下: 青
        };
        const u64 vbSize = sizeof(triangle);

        if (!m_vertexBuffer.Create(device, vbSize, D3D12_HEAP_TYPE_DEFAULT,
                                   D3D12_RESOURCE_STATE_COPY_DEST, L"TriangleVB"))
        {
            return false;
        }
        if (!m_upload.UploadBuffer(m_vertexBuffer, triangle, vbSize,
                                   D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
        {
            return false;
        }

        m_vbView.BufferLocation = m_vertexBuffer.GpuAddress();
        m_vbView.SizeInBytes    = static_cast<UINT>(vbSize);
        m_vbView.StrideInBytes  = sizeof(Vertex);
        m_vertexCount           = 3;

        LogInfo("D3D12Renderer: triangle vertex buffer uploaded (%llu bytes)",
                static_cast<unsigned long long>(vbSize));
        return true;
    }

    bool D3D12Renderer::CreatePipeline(ID3D12Device* device)
    {
        // --- ルートシグネチャ(パラメータ無し。頂点入力のみ許可) ---
        D3D12_ROOT_SIGNATURE_DESC rsDesc{};
        rsDesc.NumParameters = 0;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> rsBlob;
        ComPtr<ID3DBlob> rsError;
        const HRESULT hr = D3D12SerializeRootSignature(
            &rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &rsError);
        if (hr < 0)
        {
            if (rsError)
            {
                LogError("RootSignature serialize failed: %s",
                         static_cast<const char*>(rsError->GetBufferPointer()));
            }
            return false;
        }
        LUMINA_CHECK_HR(
            device->CreateRootSignature(0, rsBlob->GetBufferPointer(),
                                        rsBlob->GetBufferSize(),
                                        IID_PPV_ARGS(&m_rootSignature)),
            "CreateRootSignature failed");

        // --- シェーダーコンパイル(DXC, SM6) ---
        CompiledShader vs;
        CompiledShader ps;
        if (!m_shaderCompiler.Compile(m_shaderPath, L"VSMain", L"vs_6_0", vs)) return false;
        if (!m_shaderCompiler.Compile(m_shaderPath, L"PSMain", L"ps_6_0", ps)) return false;

        // --- 入力レイアウトは VS のリフレクションから自動取得 ---

        // --- ラスタライザ(カリング無し: 裏表を気にしない) ---
        D3D12_RASTERIZER_DESC raster{};
        raster.FillMode        = D3D12_FILL_MODE_SOLID;
        raster.CullMode        = D3D12_CULL_MODE_NONE;
        raster.DepthClipEnable = TRUE;

        // --- ブレンド(不透明) ---
        D3D12_BLEND_DESC blend{};
        blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        // --- PSO ---
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
        pso.pRootSignature        = m_rootSignature.Get();
        pso.VS                    = vs.ByteCode();
        pso.PS                    = ps.ByteCode();
        pso.InputLayout           = { vs.inputLayout.data(),
                                      static_cast<UINT>(vs.inputLayout.size()) };
        pso.RasterizerState       = raster;
        pso.BlendState            = blend;
        pso.DepthStencilState.DepthEnable   = FALSE;
        pso.DepthStencilState.StencilEnable = FALSE;
        pso.SampleMask            = UINT_MAX;
        pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso.NumRenderTargets      = 1;
        pso.RTVFormats[0]         = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso.SampleDesc.Count      = 1;

        LUMINA_CHECK_HR(
            device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&m_pipelineState)),
            "CreateGraphicsPipelineState failed");

        LogInfo("D3D12Renderer: pipeline created");
        return true;
    }

    void D3D12Renderer::Render(const f32 clearColor[4])
    {
        const u32 frameIndex = m_swapChain.CurrentIndex();

        // このバックバッファに以前投げた作業の完了を待つ(上書き事故を防ぐ同期)。
        m_queue.WaitForFenceValue(m_frameFenceValues[frameIndex]);

        // このフレームのアロケータとコマンドリストを記録開始状態に戻す。
        ID3D12CommandAllocator* allocator = m_allocators[frameIndex].Get();
        allocator->Reset();
        m_commandList->Reset(allocator, nullptr);

        ID3D12Resource* backBuffer = m_swapChain.CurrentBackBuffer();

        // バリア: 表示用(PRESENT) → 描画先(RENDER_TARGET)
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource   = backBuffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_commandList->ResourceBarrier(1, &barrier);

        // 描画先を設定してクリア。
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_swapChain.CurrentRTV();
        m_commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

        // --- 三角形を描画 ---
        const D3D12_VIEWPORT viewport{ 0.0f, 0.0f,
                                       static_cast<f32>(m_width), static_cast<f32>(m_height),
                                       0.0f, 1.0f };
        const D3D12_RECT scissor{ 0, 0,
                                  static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
        m_commandList->RSSetViewports(1, &viewport);
        m_commandList->RSSetScissorRects(1, &scissor);

        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        m_commandList->SetPipelineState(m_pipelineState.Get());
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vbView);
        m_commandList->DrawInstanced(m_vertexCount, 1, 0, 0);

        // バリア: 描画先(RENDER_TARGET) → 表示用(PRESENT)
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
        m_commandList->ResourceBarrier(1, &barrier);

        m_commandList->Close();

        // GPUへ実行投入 → 表示。
        ID3D12CommandList* lists[] = { m_commandList.Get() };
        m_queue.Execute(lists, 1);
        m_swapChain.Present(true);

        // このフレームの完了印を記録(次にこのバッファを使う時に待つ)。
        m_frameFenceValues[frameIndex] = m_queue.Signal();
    }

    void D3D12Renderer::Shutdown()
    {
        // GPUが全作業を終えるまで待ってから破棄する(使用中リソースの解放を防ぐ)。
        if (m_queue.Get())
        {
            m_queue.Flush();
        }

        m_pipelineState.Reset();
        m_rootSignature.Reset();
        m_shaderCompiler.Shutdown();
        m_vertexBuffer.Reset();
        m_upload.Shutdown();

        m_commandList.Reset();
        for (auto& allocator : m_allocators)
        {
            allocator.Reset();
        }
        m_swapChain.Shutdown();
        m_queue.Shutdown();
        m_device.Shutdown();
    }
}
