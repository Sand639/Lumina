#pragma once

#include "RHI/D3D12/D3D12Common.h"

// Lumina Engine - RHI / D3D12 backend
// コマンドキュー(GPUへの作業投入口)とフェンス(CPU/GPU同期)をまとめて扱う。
namespace Lumina
{
    class D3D12CommandQueue
    {
    public:
        D3D12CommandQueue() = default;
        ~D3D12CommandQueue();

        D3D12CommandQueue(const D3D12CommandQueue&) = delete;
        D3D12CommandQueue& operator=(const D3D12CommandQueue&) = delete;

        bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
        void Shutdown();

        ID3D12CommandQueue* Get() const { return m_queue.Get(); }

        // 記録済みコマンドリストをGPUへ実行投入する。
        void Execute(ID3D12CommandList* const* lists, u32 count);

        // 「ここまでの作業」を表すフェンス値をシグナル予約し、その値を返す。
        u64 Signal();

        // 指定フェンス値までGPUが進むのをCPUで待つ。
        void WaitForFenceValue(u64 value);

        // 指定フェンス値にGPUが到達済みか。
        bool IsFenceComplete(u64 value);

        // GPUが完全アイドルになるまで待つ(Signal + Wait)。
        void Flush();

    private:
        ComPtr<ID3D12CommandQueue> m_queue;
        ComPtr<ID3D12Fence>        m_fence;
        HANDLE                     m_fenceEvent    = nullptr;
        u64                        m_nextFenceValue = 1;  // 0 は「未投入」を表すので 1 から
    };
}
