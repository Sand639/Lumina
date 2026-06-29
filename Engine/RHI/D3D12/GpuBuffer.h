#pragma once

#include "RHI/D3D12/D3D12Common.h"

// Lumina Engine - RHI / D3D12 backend
// GPUバッファ(頂点/インデックス/定数など)を RAII で包む。
// 生の ID3D12Resource を上位に漏らさず、寿命を型に紐づける。
namespace Lumina
{
    class GpuBuffer
    {
    public:
        GpuBuffer() = default;
        ~GpuBuffer();

        GpuBuffer(const GpuBuffer&) = delete;
        GpuBuffer& operator=(const GpuBuffer&) = delete;
        GpuBuffer(GpuBuffer&& other) noexcept;
        GpuBuffer& operator=(GpuBuffer&& other) noexcept;

        // バッファを確保する。heapType=DEFAULT(GPU専用) / UPLOAD(CPU書込可)。
        bool Create(ID3D12Device* device, u64 sizeBytes, D3D12_HEAP_TYPE heapType,
                    D3D12_RESOURCE_STATES initialState, const wchar_t* debugName = nullptr);
        void Reset();

        // UPLOAD ヒープのときだけ有効: CPUからデータを書き込む。
        bool Upload(const void* data, u64 size);

        ID3D12Resource*           Get()        const { return m_resource.Get(); }
        D3D12_GPU_VIRTUAL_ADDRESS GpuAddress() const;
        u64                       Size()       const { return m_size; }

    private:
        ComPtr<ID3D12Resource> m_resource;
        u64                    m_size     = 0;
        D3D12_HEAP_TYPE        m_heapType = D3D12_HEAP_TYPE_DEFAULT;
    };
}
