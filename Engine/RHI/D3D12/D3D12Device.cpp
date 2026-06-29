#include "RHI/D3D12/D3D12Device.h"

#include <d3d12sdklayers.h>  // ID3D12Debug

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace Lumina
{
    D3D12Device::~D3D12Device()
    {
        Shutdown();
    }

    bool D3D12Device::Initialize()
    {
        // 順番が重要: デバッグレイヤーはデバイス生成より前に有効化する。
        EnableDebugLayer();  // 失敗しても続行可(致命ではない)

        if (!CreateFactory()) return false;
        if (!PickAdapter())   return false;
        if (!CreateDevice())  return false;

        LogInfo("D3D12Device: initialized");
        return true;
    }

    void D3D12Device::Shutdown()
    {
        // ComPtr が参照解放するので明示破棄は不要だが、順序を明確にしておく。
        m_device.Reset();
        m_adapter.Reset();
        m_factory.Reset();
    }

    bool D3D12Device::EnableDebugLayer()
    {
#ifdef _DEBUG
        ComPtr<ID3D12Debug> debug;
        if (D3D12GetDebugInterface(IID_PPV_ARGS(&debug)) >= 0)
        {
            debug->EnableDebugLayer();
            LogInfo("D3D12Device: debug layer enabled");

            // GPUベースの検証(より厳密。重いが Phase 1 では有用)。
            ComPtr<ID3D12Debug1> debug1;
            if (debug.As(&debug1) >= 0)
            {
                debug1->SetEnableGPUBasedValidation(TRUE);
                LogInfo("D3D12Device: GPU-based validation enabled");
            }
            return true;
        }
        LogWarn("D3D12Device: debug layer not available");
        return false;
#else
        return true;
#endif
    }

    bool D3D12Device::CreateFactory()
    {
        UINT flags = 0;
#ifdef _DEBUG
        flags |= DXGI_CREATE_FACTORY_DEBUG;  // DXGI 側の検証も有効化
#endif
        LUMINA_CHECK_HR(CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_factory)),
                        "CreateDXGIFactory2 failed");
        return true;
    }

    bool D3D12Device::PickAdapter()
    {
        // 高性能(ディスクリートGPU)優先で列挙し、最初に D3D12 対応したものを採用。
        for (UINT i = 0;; ++i)
        {
            ComPtr<IDXGIAdapter1> adapter;
            const HRESULT hr = m_factory->EnumAdapterByGpuPreference(
                i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
            if (hr == DXGI_ERROR_NOT_FOUND)
            {
                break;  // もうアダプタが無い
            }

            DXGI_ADAPTER_DESC1 desc{};
            adapter->GetDesc1(&desc);

            // ソフトウェアアダプタ(WARP)は除外。
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            // 実際にデバイスを作れるか試す(生成はまだしない、対応確認のみ)。
            if (D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                  __uuidof(ID3D12Device), nullptr) >= 0)
            {
                m_adapter = adapter;
                LogInfo("D3D12Device: adapter selected: %ls (VRAM %llu MB)",
                        desc.Description,
                        static_cast<unsigned long long>(desc.DedicatedVideoMemory / (1024 * 1024)));
                return true;
            }
        }

        LogError("D3D12Device: no D3D12-capable adapter found");
        return false;
    }

    bool D3D12Device::CreateDevice()
    {
        // 12_0 を狙い、ダメなら 11_0 にフォールバック。
        const D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_0,
        };

        for (const D3D_FEATURE_LEVEL level : levels)
        {
            if (D3D12CreateDevice(m_adapter.Get(), level, IID_PPV_ARGS(&m_device)) >= 0)
            {
                LogInfo("D3D12Device: device created (feature level 0x%04X)",
                        static_cast<unsigned>(level));
                return true;
            }
        }

        LogError("D3D12Device: D3D12CreateDevice failed for all feature levels");
        return false;
    }
}
