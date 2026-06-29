#include "RHI/D3D12/UploadContext.h"
#include "RHI/D3D12/D3D12CommandQueue.h"

namespace Lumina
{
    bool UploadContext::Initialize(ID3D12Device* device, D3D12CommandQueue* queue)
    {
        m_device = device;
        m_queue  = queue;

        LUMINA_CHECK_HR(
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                           IID_PPV_ARGS(&m_allocator)),
            "UploadContext: CreateCommandAllocator failed");

        LUMINA_CHECK_HR(
            device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                      m_allocator.Get(), nullptr, IID_PPV_ARGS(&m_list)),
            "UploadContext: CreateCommandList failed");
        m_list->Close();

        LogInfo("UploadContext: initialized");
        return true;
    }

    void UploadContext::Shutdown()
    {
        m_list.Reset();
        m_allocator.Reset();
        m_device = nullptr;
        m_queue  = nullptr;
    }

    bool UploadContext::UploadBuffer(GpuBuffer& dest, const void* data, u64 size,
                                     D3D12_RESOURCE_STATES afterState)
    {
        // 1) 一時 UPLOAD バッファに CPU から書き込む。
        GpuBuffer staging;
        if (!staging.Create(m_device, size, D3D12_HEAP_TYPE_UPLOAD,
                            D3D12_RESOURCE_STATE_GENERIC_READ, L"UploadStaging"))
        {
            return false;
        }
        if (!staging.Upload(data, size))
        {
            return false;
        }

        // 2) コピーコマンドを記録する。
        m_allocator->Reset();
        m_list->Reset(m_allocator.Get(), nullptr);

        m_list->CopyBufferRegion(dest.Get(), 0, staging.Get(), 0, size);

        // 3) コピー先を COPY_DEST から目的の状態へ遷移。
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource   = dest.Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter  = afterState;
        m_list->ResourceBarrier(1, &barrier);

        m_list->Close();

        // 4) 実行して完了を待つ(同期アップロード)。
        ID3D12CommandList* lists[] = { m_list.Get() };
        m_queue->Execute(lists, 1);
        m_queue->Flush();  // 完了を待つので staging はこの後破棄して安全

        return true;
    }
}
