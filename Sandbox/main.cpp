#include "Core/Log.h"
#include "Platform/Application.h"

// Lumina Sandbox - Engine を使う実行アプリ。
// Application を継承し、必要なフックだけを実装する。
class SandboxApp : public Lumina::Application
{
public:
    SandboxApp()
        : Application(Config{ L"Lumina - Sandbox", 1280, 720 })
    {
    }

protected:
    bool OnInit() override
    {
        Lumina::LogInfo("SandboxApp: initialized");
        return true;
    }

    void OnUpdate(float /*dt*/) override
    {
        // Phase 0 ではまだ何もしない。
    }

    void OnRender() override
    {
        // Phase 1 でここに描画(クリア->Present)が入る。
    }

    void OnShutdown() override
    {
        Lumina::LogInfo("SandboxApp: shutting down");
    }
};

int main()
{
    SandboxApp app;
    return app.Run();
}
