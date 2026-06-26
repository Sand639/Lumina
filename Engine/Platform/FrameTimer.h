#pragma once

#include "Core/Types.h"

// Lumina Engine - Platform
// 高分解能タイマー。毎フレーム Tick() を呼び、前フレームからの経過秒(delta)を得る。
namespace Lumina
{
    class FrameTimer
    {
    public:
        FrameTimer();

        // 1フレームに1回呼ぶ。delta を更新する。
        void Tick();

        f32 DeltaSeconds() const { return m_delta; }
        f32 Fps()          const { return m_delta > 0.0f ? 1.0f / m_delta : 0.0f; }

    private:
        i64 m_frequency = 0;  // カウンタ周波数(1秒あたり)
        i64 m_last      = 0;  // 前回 Tick 時のカウンタ値
        f32 m_delta     = 0.0f;
    };
}
