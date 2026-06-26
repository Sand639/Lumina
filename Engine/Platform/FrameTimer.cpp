#include "Platform/FrameTimer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Lumina
{
    FrameTimer::FrameTimer()
    {
        LARGE_INTEGER freq{};
        LARGE_INTEGER now{};
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&now);
        m_frequency = freq.QuadPart;
        m_last      = now.QuadPart;
    }

    void FrameTimer::Tick()
    {
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);

        const i64 elapsed = now.QuadPart - m_last;
        m_last  = now.QuadPart;
        m_delta = static_cast<f32>(static_cast<f64>(elapsed) / static_cast<f64>(m_frequency));
    }
}
