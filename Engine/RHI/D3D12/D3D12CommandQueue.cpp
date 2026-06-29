#include "RHI/D3D12/D3D12CommandQueue.h"

namespace Lumina
{
    D3D12CommandQueue::~D3D12CommandQueue()
    {
        Shutdown();
    }

    bool D3D12CommandQueue::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type  = type;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        LUMINA_CHECK_HR(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)),
                        "CreateCommandQueue failed");

        LUMINA_CHECK_HR(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)),
                        "CreateFence failed");

        // フェンス到達を待つための同期イベント。
        m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!m_fenceEvent)
        {
            LogError("CreateEventW failed for fence");
            return false;
        }

        LogInfo("D3D12CommandQueue: initialized");
        return true;
    }

    void D3D12CommandQueue::Shutdown()
    {
        if (m_fenceEvent)
        {
            CloseHandle(m_fenceEvent);
            m_fenceEvent = nullptr;
        }
        m_fence.Reset();
        m_queue.Reset();
    }

    void D3D12CommandQueue::Execute(ID3D12CommandList* const* lists, u32 count)
    {
        m_queue->ExecuteCommandLists(count, lists);
    }

    u64 D3D12CommandQueue::Signal()
    {
        const u64 value = m_nextFenceValue++;
        m_queue->Signal(m_fence.Get(), value);
        return value;
    }

    bool D3D12CommandQueue::IsFenceComplete(u64 value)
    {
        return m_fence->GetCompletedValue() >= value;
    }

    void D3D12CommandQueue::WaitForFenceValue(u64 value)
    {
        if (!IsFenceComplete(value))
        {
            m_fence->SetEventOnCompletion(value, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    void D3D12CommandQueue::Flush()
    {
        WaitForFenceValue(Signal());
    }
}
