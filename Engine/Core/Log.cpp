#include "Core/Log.h"

#include <cstdarg>
#include <cstdio>

namespace Lumina
{
    void LogInfo(const char* fmt, ...)
    {
        std::printf("[Lumina] ");

        va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);

        std::printf("\n");
    }
}
