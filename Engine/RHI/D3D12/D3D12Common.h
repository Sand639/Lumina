#pragma once

// Lumina Engine - RHI / D3D12 backend (concrete)
// D3D12 実装で共通して使うインクルードとヘルパをまとめる。
// ※ Phase 1 は「動かしてから抽象化」方針のため、まず具象(D3D12)を直接書く。
//    後で RHI/ 配下に IDevice 等のインターフェースを抽出する。

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include "Core/Types.h"
#include "Core/Log.h"

namespace Lumina
{
    // COMオブジェクトの参照カウントを自動管理するスマートポインタ。
    template <class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;
}

// HRESULT を判定し、失敗なら原因をログして false を return するヘルパ。
// bool を返す初期化関数の中で使う想定。
#define LUMINA_CHECK_HR(expr, msg)                                              \
    do                                                                          \
    {                                                                           \
        const HRESULT _hr = (expr);                                             \
        if (_hr < 0)                                                            \
        {                                                                       \
            ::Lumina::LogError("%s (hr=0x%08X) | %s:%d",                        \
                               (msg), static_cast<unsigned>(_hr),              \
                               __FILE__, __LINE__);                             \
            return false;                                                       \
        }                                                                       \
    } while (0)
