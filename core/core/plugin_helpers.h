#pragma once

#include <filesystem>

namespace teamspeak
{
namespace plugin
{
    enum class Path
    {
        App = 0,
        Config,
        Resources,
        PluginIni
    };

    std::filesystem::path get_path(Path path);
}  // namespace plugin
}  // namespace teamspeak
