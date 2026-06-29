#pragma once

#include "RHI/D3D12/D3D12Common.h"

// Lumina Engine - RHI / D3D12 backend
// D3D12 デバイスと、それを作るための DXGI ファクトリ/アダプタを保持する。
// 「GPUを表す中心オブジェクト」。コマンド・リソース・スワップチェーンの生成元になる。
namespace Lumina
{
    class D3D12Device
    {
    public:
        D3D12Device() = default;
        ~D3D12Device();

        D3D12Device(const D3D12Device&) = delete;
        D3D12Device& operator=(const D3D12Device&) = delete;

        // デバッグレイヤー有効化→ファクトリ生成→アダプタ選択→デバイス生成。
        bool Initialize();
        void Shutdown();

        ID3D12Device*  Device()  const { return m_device.Get(); }
        IDXGIFactory6* Factory() const { return m_factory.Get(); }
        IDXGIAdapter1* Adapter() const { return m_adapter.Get(); }

    private:
        bool EnableDebugLayer();   // Debug ビルドのみ実効
        bool CreateFactory();
        bool PickAdapter();        // 高性能GPUを選ぶ(ソフトウェアは除外)
        bool CreateDevice();

        ComPtr<IDXGIFactory6> m_factory;
        ComPtr<IDXGIAdapter1> m_adapter;
        ComPtr<ID3D12Device>  m_device;
    };
}
