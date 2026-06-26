#pragma once

// Lumina Engine - Core
// Phase 0 の最小ログ。後で本実装（レベル/出力先/フォーマット）に差し替える。

namespace Lumina
{
    // フォーマット付きでログを1行出力する。
    void LogInfo(const char* fmt, ...);
}
