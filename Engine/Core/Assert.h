#pragma once

#include "Core/Log.h"

// Lumina Engine - Core
// デバッグ時のみ有効なアサート。条件が偽ならログを出してデバッガで停止する。
// Release では完全に消える(((void)0))。
#ifdef _DEBUG
    #define LUMINA_ASSERT(cond, msg)                                            \
        do                                                                      \
        {                                                                       \
            if (!(cond))                                                        \
            {                                                                   \
                ::Lumina::LogError("ASSERT failed: (%s) | %s | %s:%d",          \
                                   #cond, (msg), __FILE__, __LINE__);           \
                __debugbreak();                                                 \
            }                                                                   \
        } while (0)
#else
    #define LUMINA_ASSERT(cond, msg) ((void)0)
#endif
