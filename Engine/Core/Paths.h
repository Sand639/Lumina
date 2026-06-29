#pragma once

#include <string>

// Lumina Engine - Core
// 実行ファイル位置を基準に、アセット等の絶対パスを解決する。
namespace Lumina
{
    // 実行ファイルのあるディレクトリ(末尾に区切り無し)。
    std::wstring GetExecutableDir();

    // リポジトリの Assets ディレクトリ(末尾に \\)。
    // ビルド出力 <repo>\x64\<Config>\ から2つ上がって解決する。
    std::wstring GetAssetsDir();
}
