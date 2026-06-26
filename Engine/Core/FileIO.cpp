#include "Core/FileIO.h"
#include "Core/Log.h"

#include <fstream>

namespace Lumina
{
    bool ReadFileBinary(std::string_view path, std::vector<u8>& out)
    {
        std::ifstream file(std::string(path), std::ios::binary | std::ios::ate);
        if (!file)
        {
            LogError("ReadFileBinary: open failed: %.*s",
                     static_cast<int>(path.size()), path.data());
            return false;
        }

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        out.resize(static_cast<usize>(size));
        if (size > 0 && !file.read(reinterpret_cast<char*>(out.data()), size))
        {
            LogError("ReadFileBinary: read failed: %.*s",
                     static_cast<int>(path.size()), path.data());
            return false;
        }
        return true;
    }

    bool ReadFileText(std::string_view path, std::string& out)
    {
        std::ifstream file(std::string(path), std::ios::ate);
        if (!file)
        {
            LogError("ReadFileText: open failed: %.*s",
                     static_cast<int>(path.size()), path.data());
            return false;
        }

        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        out.resize(static_cast<usize>(size));
        if (size > 0)
        {
            file.read(out.data(), size);
            // テキストモードでは改行変換で実読込が短くなり得るのでサイズを詰める。
            out.resize(static_cast<usize>(file.gcount()));
        }
        return true;
    }
}
