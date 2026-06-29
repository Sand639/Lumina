#include "Core/Log.h"
#include "Platform/Application.h"
#include "RHI/D3D12/D3D12Device.h"

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

        // Phase 1 増分1: D3D12 デバイスを起動する(まだ描画はしない)。
        if (!m_device.Initialize())
        {
            return false;
        }
        return true;
    }

    void OnUpdate(float /*dt*/) override
    {
        // Phase 0 ではまだ何もしない。
    }

    void OnRender() override
    {
        // Phase 1 増分2 でここに「クリア→Present」が入る。
    }

    void OnShutdown() override
    {
        m_device.Shutdown();
        Lumina::LogInfo("SandboxApp: shutting down");
    }

private:
    Lumina::D3D12Device m_device;
};

int main()
{
    SandboxApp app;
    return app.Run();
}
