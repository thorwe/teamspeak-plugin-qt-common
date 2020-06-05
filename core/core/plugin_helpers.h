#pragma once

#include <filesystem>

namespace teamspeak::plugin
{
    enum class Path {
        Config = 0,
        Resources,
        PluginIni
    };

    std::filesystem::path get_path(Path path);

}
