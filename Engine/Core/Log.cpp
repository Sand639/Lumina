#include "Core/Log.h"

#include <cstdarg>
#include <cstdio>

namespace Lumina
{
    namespace
    {
        // 共通本体: プレフィックス + 可変長フォーマット + 改行。
        void LogV(const char* level, const char* fmt, va_list args)
        {
            std::printf("[Lumina][%s] ", level);
            std::vprintf(fmt, args);
            std::printf("\n");
        }
    }

    void LogInfo(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        LogV("INFO", fmt, args);
        va_end(args);
    }

    void LogWarn(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        LogV("WARN", fmt, args);
        va_end(args);
    }

    void LogError(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        LogV("ERROR", fmt, args);
        va_end(args);
    }
}
