#include "Platform/Window.h"
#include "Core/Log.h"

namespace Lumina
{
    namespace
    {
        constexpr const wchar_t* kWindowClassName = L"LuminaWindowClass";
    }

    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(const wchar_t* title, i32 width, i32 height)
    {
        // DPI認識(Per-Monitor V2)にする。クライアント領域=物理ピクセルになり、
        // バックバッファ解像度と画面が1:1で一致する(高DPI環境でのボケ/ズレ防止)。
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        const HINSTANCE hInstance = GetModuleHandleW(nullptr);

        WNDCLASSEXW wc{};
        wc.cbSize        = sizeof(WNDCLASSEXW);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = &Window::WndProc;
        wc.hInstance     = hInstance;
        wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
        wc.lpszClassName = kWindowClassName;

        // 既に登録済み(2回目の生成)でも続行できるよう、登録失敗は無視する。
        RegisterClassExW(&wc);

        // クライアント領域が指定サイズになるよう、枠ぶんを足す。
        RECT rect{ 0, 0, width, height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowExW(
            0,
            kWindowClassName,
            title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left,
            rect.bottom - rect.top,
            nullptr, nullptr, hInstance, nullptr);

        if (!m_hwnd)
        {
            LogError("Window::Create failed (CreateWindowExW)");
            return false;
        }

        m_width  = width;
        m_height = height;

        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        return true;
    }

    void Window::Destroy()
    {
        if (m_hwnd)
        {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    bool Window::PumpMessages()
    {
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return true;
    }

    LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        }
    }
}
