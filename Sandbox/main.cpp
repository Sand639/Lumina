#include "Core/Log.h"
#include "Platform/Application.h"
#include "RHI/D3D12/D3D12Renderer.h"

#include <cmath>

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
        return m_renderer.Initialize(GetWindow().Handle(),
                                     static_cast<Lumina::u32>(GetWindow().Width()),
                                     static_cast<Lumina::u32>(GetWindow().Height()));
    }

    void OnUpdate(float dt) override
    {
        m_time += dt;
    }

    void OnRender() override
    {
        // 時間で色を周期変化させ、毎フレーム更新されていることを目視できるようにする。
        const float color[4] = {
            0.5f + 0.5f * std::sin(m_time),
            0.5f + 0.5f * std::sin(m_time + 2.094f),  // +120°
            0.5f + 0.5f * std::sin(m_time + 4.188f),  // +240°
            1.0f,
        };
        m_renderer.Render(color);
    }

    void OnShutdown() override
    {
        m_renderer.Shutdown();
        Lumina::LogInfo("SandboxApp: shutting down");
    }

private:
    Lumina::D3D12Renderer m_renderer;
    float                 m_time = 0.0f;
};

int main()
{
    SandboxApp app;
    return app.Run();
}
