#include "RHI/D3D12/D3D12Renderer.h"

namespace Lumina
{
    D3D12Renderer::~D3D12Renderer()
    {
        Shutdown();
    }

    bool D3D12Renderer::Initialize(HWND hwnd, u32 width, u32 height)
    {
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

        LogInfo("D3D12Renderer: initialized");
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
