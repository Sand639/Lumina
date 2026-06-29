#pragma once

#include "RHI/D3D12/D3D12Common.h"
#include "RHI/D3D12/GpuBuffer.h"

// Lumina Engine - RHI / D3D12 backend
// CPUのデータを GPU専用(DEFAULT)バッファへ転送するアップロード機構。
// 仕組み: 一時的な UPLOAD バッファに書き → コピーコマンドで DEFAULT へ → GPU完了待ち。
// ※ Phase 2 では「一回限りの同期アップロード」。リングバッファ化は必要になった段階で行う。
namespace Lumina
{
    class D3D12CommandQueue;

    class UploadContext
    {
    public:
        bool Initialize(ID3D12Device* device, D3D12CommandQueue* queue);
        void Shutdown();

        // dest は DEFAULT ヒープ・COPY_DEST 状態で作っておくこと。
        // 転送後、dest を afterState へ遷移させる。
        bool UploadBuffer(GpuBuffer& dest, const void* data, u64 size,
                          D3D12_RESOURCE_STATES afterState);

    private:
        ID3D12Device*      m_device = nullptr;
        D3D12CommandQueue* m_queue  = nullptr;

        ComPtr<ID3D12CommandAllocator>    m_allocator;
        ComPtr<ID3D12GraphicsCommandList> m_list;
    };
}
