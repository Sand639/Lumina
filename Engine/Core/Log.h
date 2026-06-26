#pragma once

// Lumina Engine - Core
// レベル付きの最小ログ。Phase 0 では printf ベース。
// 後でファイル出力やフォーマット強化に差し替えられるよう、呼び出し側はこのIFだけ使う。
namespace Lumina
{
    void LogInfo(const char* fmt, ...);
    void LogWarn(const char* fmt, ...);
    void LogError(const char* fmt, ...);
}
