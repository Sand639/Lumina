#pragma once

#include "Core/Types.h"
#include "Platform/Window.h"
#include "Platform/FrameTimer.h"

// Lumina Engine - Platform
// アプリケーションの骨格。利用側(Sandbox)はこれを継承して On* を実装する。
// ライフサイクル: Run() の中で Init -> (Update/Render ループ) -> Shutdown。
namespace Lumina
{
    class Application
    {
    public:
        struct Config
        {
            const wchar_t* title  = L"Lumina";
            i32            width  = 1280;
            i32            height = 720;
        };

        explicit Application(const Config& config);
        virtual ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // メインループを回す。終了コードを返す(正常:0)。
        int Run();

    protected:
        // 派生側のフック。デフォルトは何もしない。
        virtual bool OnInit()              { return true; }
        virtual void OnUpdate(f32 /*dt*/)  {}
        virtual void OnRender()            {}
        virtual void OnShutdown()          {}

        const Window& GetWindow() const { return m_window; }

    private:
        Config      m_config;
        Window      m_window;
        FrameTimer  m_timer;
    };
}
