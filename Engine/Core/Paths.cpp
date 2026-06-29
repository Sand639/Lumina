#include "Core/Paths.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace Lumina
{
    std::wstring GetExecutableDir()
    {
        wchar_t buffer[MAX_PATH]{};
        const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        std::wstring path(buffer, len);

        const size_t slash = path.find_last_of(L"\\/");
        if (slash != std::wstring::npos)
        {
            path.resize(slash);  // ファイル名を落とす
        }
        return path;
    }

    std::wstring GetAssetsDir()
    {
        // <repo>\x64\<Config>\Sandbox.exe → 2つ上が <repo>。
        const std::wstring raw = GetExecutableDir() + L"\\..\\..\\Assets\\";

        // ".." を畳んで正規化する。
        wchar_t full[MAX_PATH]{};
        const DWORD len = GetFullPathNameW(raw.c_str(), MAX_PATH, full, nullptr);
        if (len > 0 && len < MAX_PATH)
        {
            return std::wstring(full, len);
        }
        return raw;
    }
}
