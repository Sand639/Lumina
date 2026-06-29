#pragma once

#include "RHI/D3D12/D3D12Common.h"

// Lumina Engine - RHI / D3D12 backend
// スワップチェーン(表示用バックバッファ群)と、その描画先ビュー(RTV)を管理する。
namespace Lumina
{
    class D3D12SwapChain
    {
    public:
        static constexpr u32 kBackBufferCount = 3;  // トリプルバッファ

        D3D12SwapChain() = default;
        ~D3D12SwapChain();

        D3D12SwapChain(const D3D12SwapChain&) = delete;
        D3D12SwapChain& operator=(const D3D12SwapChain&) = delete;

        bool Initialize(IDXGIFactory6* factory, ID3D12CommandQueue* queue,
                        ID3D12Device* device, HWND hwnd, u32 width, u32 height);
        void Shutdown();

        void Present(bool vsync);

        // 今から描くべきバックバッファのインデックス。
        u32 CurrentIndex() const { return m_swapChain->GetCurrentBackBufferIndex(); }

        ID3D12Resource* CurrentBackBuffer() const { return m_backBuffers[CurrentIndex()].Get(); }

        // 現在のバックバッファに対応するRTV(CPUディスクリプタハンドル)。
        D3D12_CPU_DESCRIPTOR_HANDLE CurrentRTV() const;

    private:
        bool CreateRenderTargets(ID3D12Device* device);

        ComPtr<IDXGISwapChain3>      m_swapChain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource>       m_backBuffers[kBackBufferCount];
        u32                          m_rtvDescriptorSize = 0;
    };
}
