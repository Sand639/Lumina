#pragma once

#include "Core/Types.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// Lumina Engine - Platform
// Win32 ウィンドウのRAIIラッパ。Platform層はOS依存を許容する。
namespace Lumina
{
    class Window
    {
    public:
        Window() = default;
        ~Window();

        // コピー禁止(HWNDの二重破棄を防ぐ)。
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // ウィンドウを生成して表示する。成功で true。
        bool Create(const wchar_t* title, i32 width, i32 height);
        void Destroy();

        // 溜まったメッセージを処理する。WM_QUIT を受け取ったら false。
        bool PumpMessages();

        HWND Handle() const { return m_hwnd; }
        i32  Width()  const { return m_width; }
        i32  Height() const { return m_height; }

    private:
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        HWND m_hwnd   = nullptr;
        i32  m_width  = 0;
        i32  m_height = 0;
    };
}
