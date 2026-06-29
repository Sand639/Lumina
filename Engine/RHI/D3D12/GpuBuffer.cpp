#include "RHI/D3D12/GpuBuffer.h"

#include <cstring>
#include <utility>

namespace Lumina
{
    GpuBuffer::~GpuBuffer()
    {
        Reset();
    }

    GpuBuffer::GpuBuffer(GpuBuffer&& other) noexcept
        : m_resource(std::move(other.m_resource))
        , m_size(other.m_size)
        , m_heapType(other.m_heapType)
    {
        other.m_size = 0;
    }

    GpuBuffer& GpuBuffer::operator=(GpuBuffer&& other) noexcept
    {
        if (this != &other)
        {
            m_resource = std::move(other.m_resource);
            m_size     = other.m_size;
            m_heapType = other.m_heapType;
            other.m_size = 0;
        }
        return *this;
    }

    bool GpuBuffer::Create(ID3D12Device* device, u64 sizeBytes, D3D12_HEAP_TYPE heapType,
                           D3D12_RESOURCE_STATES initialState, const wchar_t* debugName)
    {
        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = heapType;

        D3D12_RESOURCE_DESC desc{};
        desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width              = sizeBytes;
        desc.Height             = 1;
        desc.DepthOrArraySize   = 1;
        desc.MipLevels          = 1;
        desc.Format             = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count   = 1;
        desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;  // バッファは必ずこれ
        desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

        LUMINA_CHECK_HR(
            device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
                                            initialState, nullptr,
                                            IID_PPV_ARGS(&m_resource)),
            "CreateCommittedResource(buffer) failed");

        m_size     = sizeBytes;
        m_heapType = heapType;

        if (debugName)
        {
            m_resource->SetName(debugName);
        }
        return true;
    }

    bool GpuBuffer::Upload(const void* data, u64 size)
    {
        if (m_heapType != D3D12_HEAP_TYPE_UPLOAD)
        {
            LogError("GpuBuffer::Upload called on non-UPLOAD heap buffer");
            return false;
        }
        if (size > m_size)
        {
            LogError("GpuBuffer::Upload size overflow (%llu > %llu)",
                     static_cast<unsigned long long>(size),
                     static_cast<unsigned long long>(m_size));
            return false;
        }

        void* mapped = nullptr;
        const D3D12_RANGE readRange{ 0, 0 };  // CPUは読まない
        LUMINA_CHECK_HR(m_resource->Map(0, &readRange, &mapped), "Buffer Map failed");
        std::memcpy(mapped, data, static_cast<size_t>(size));
        m_resource->Unmap(0, nullptr);
        return true;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GpuBuffer::GpuAddress() const
    {
        return m_resource ? m_resource->GetGPUVirtualAddress() : 0;
    }

    void GpuBuffer::Reset()
    {
        m_resource.Reset();
        m_size = 0;
    }
}
