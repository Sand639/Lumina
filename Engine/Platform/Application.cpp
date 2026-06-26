#include "Platform/Application.h"
#include "Core/Log.h"

#include <chrono>
#include <thread>

#include <timeapi.h>      // timeBeginPeriod / timeEndPeriod
#pragma comment(lib, "winmm.lib")

namespace Lumina
{
    Application::Application(const Config& config)
        : m_config(config)
    {
    }

    int Application::Run()
    {
        if (!m_window.Create(m_config.title, m_config.width, m_config.height))
        {
            LogError("Application: window creation failed");
            return -1;
        }
        LogInfo("Application: window created (%dx%d)", m_config.width, m_config.height);

        if (!OnInit())
        {
            LogError("Application: OnInit failed");
            return -1;
        }

        // Phase 0 はまだ Present が無いので、ビジーループ防止に約60FPSへ制限する。
        // 既定の Sleep 分解能(約15.6ms)では粗すぎるため、1msに上げて正確に刻む。
        timeBeginPeriod(1);

        using clock = std::chrono::steady_clock;
        const auto targetFrame = std::chrono::duration<f64>(1.0 / 60.0);

        f32 fpsAccum   = 0.0f;
        i32 frameCount = 0;

        auto frameStart = clock::now();
        m_timer = FrameTimer{};  // ループ直前で基準を取り直す

        while (m_window.PumpMessages())
        {
            m_timer.Tick();
            const f32 dt = m_timer.DeltaSeconds();

            OnUpdate(dt);
            OnRender();

            // 1秒ごとに実測FPSをログ。
            fpsAccum += dt;
            ++frameCount;
            if (fpsAccum >= 1.0f)
            {
                LogInfo("FPS: %d", frameCount);
                fpsAccum  -= 1.0f;
                frameCount = 0;
            }

            // フレーム余り時間をスリープして約60FPSに制限。
            const auto elapsed = clock::now() - frameStart;
            if (elapsed < targetFrame)
            {
                std::this_thread::sleep_for(targetFrame - elapsed);
            }
            frameStart = clock::now();
        }

        timeEndPeriod(1);

        OnShutdown();
        m_window.Destroy();
        LogInfo("Application: shutdown complete");
        return 0;
    }
}
