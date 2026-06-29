#include "RHI/D3D12/D3D12SwapChain.h"

namespace Lumina
{
    D3D12SwapChain::~D3D12SwapChain()
    {
        Shutdown();
    }

    bool D3D12SwapChain::Initialize(IDXGIFactory6* factory, ID3D12CommandQueue* queue,
                                    ID3D12Device* device, HWND hwnd, u32 width, u32 height)
    {
        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width            = width;
        desc.Height           = height;
        desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;                                  // MSAAなし
        desc.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount      = kBackBufferCount;
        desc.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;      // D3D12必須のフリップモデル
        desc.AlphaMode        = DXGI_ALPHA_MODE_UNSPECIFIED;
        desc.Scaling          = DXGI_SCALING_STRETCH;

        ComPtr<IDXGISwapChain1> sc1;
        LUMINA_CHECK_HR(
            factory->CreateSwapChainForHwnd(queue, hwnd, &desc, nullptr, nullptr, &sc1),
            "CreateSwapChainForHwnd failed");

        // Alt+Enter による勝手な全画面切替を無効化(自前管理のため)。
        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        LUMINA_CHECK_HR(sc1.As(&m_swapChain), "QueryInterface IDXGISwapChain3 failed");

        // バックバッファ数分の RTV を置くディスクリプタヒープ。
        D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
        heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDesc.NumDescriptors = kBackBufferCount;
        heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;  // RTVはシェーダ非可視
        LUMINA_CHECK_HR(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap)),
                        "CreateDescriptorHeap(RTV) failed");

        m_rtvDescriptorSize =
            device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        if (!CreateRenderTargets(device))
        {
            return false;
        }

        LogInfo("D3D12SwapChain: initialized (%ux%u, %u buffers)",
                width, height, kBackBufferCount);
        return true;
    }

    bool D3D12SwapChain::CreateRenderTargets(ID3D12Device* device)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        for (u32 i = 0; i < kBackBufferCount; ++i)
        {
            LUMINA_CHECK_HR(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i])),
                            "SwapChain GetBuffer failed");
            device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, handle);
            handle.ptr += m_rtvDescriptorSize;
        }
        return true;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12SwapChain::CurrentRTV() const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(CurrentIndex()) * m_rtvDescriptorSize;
        return handle;
    }

    void D3D12SwapChain::Present(bool vsync)
    {
        m_swapChain->Present(vsync ? 1 : 0, 0);
    }

    void D3D12SwapChain::Shutdown()
    {
        for (auto& buffer : m_backBuffers)
        {
            buffer.Reset();
        }
        m_rtvHeap.Reset();
        m_swapChain.Reset();
    }
}
