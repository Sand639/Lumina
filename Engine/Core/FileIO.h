#pragma once

#include "Core/Types.h"

#include <string>
#include <string_view>
#include <vector>

// Lumina Engine - Core
// 最小ファイル入出力。Phase 3 のシェーダー読み込み等で使う土台。
namespace Lumina
{
    // バイナリ全読み。成功で true、out に内容を格納。
    bool ReadFileBinary(std::string_view path, std::vector<u8>& out);

    // テキスト全読み。成功で true、out に内容を格納。
    bool ReadFileText(std::string_view path, std::string& out);
}
