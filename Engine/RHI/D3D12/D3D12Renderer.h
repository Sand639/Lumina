#pragma once

#include "RHI/D3D12/D3D12Device.h"
#include "RHI/D3D12/D3D12CommandQueue.h"
#include "RHI/D3D12/D3D12SwapChain.h"

// Lumina Engine - RHI / D3D12 backend
// デバイス/キュー/スワップチェーンを束ね、1フレームの描画を回す最上位オブジェクト。
// Phase 1 ではバックバッファを単色クリアして Present するだけ。
namespace Lumina
{
    class D3D12Renderer
    {
    public:
        D3D12Renderer() = default;
        ~D3D12Renderer();

        D3D12Renderer(const D3D12Renderer&) = delete;
        D3D12Renderer& operator=(const D3D12Renderer&) = delete;

        bool Initialize(HWND hwnd, u32 width, u32 height);
        void Shutdown();

        // 指定色でバックバッファをクリアして表示する(RGBA, 0..1)。
        void Render(const f32 clearColor[4]);

    private:
        static constexpr u32 kFrameCount = D3D12SwapChain::kBackBufferCount;

        D3D12Device       m_device;
        D3D12CommandQueue m_queue;
        D3D12SwapChain    m_swapChain;

        // フレームを並行させるため(frames-in-flight)、バッファ数ぶん用意する。
        ComPtr<ID3D12CommandAllocator>    m_allocators[kFrameCount];
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        u64                               m_frameFenceValues[kFrameCount] = {};
    };
}
