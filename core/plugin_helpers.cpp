#include "core/plugin_helpers.h"

#include "ts3_functions.h"
#include "plugin.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace teamspeak::plugin
{

constexpr const int32_t kPathBufferSize = 512;

std::filesystem::path get_path(Path path)
{
    std::filesystem::path result;

    switch (path)
    {
    case Path::Config:
    {
        auto path_ = std::array<char, kPathBufferSize>();
        ts3Functions.getConfigPath(path_.data(), kPathBufferSize);
        result = path_.data();
        break;
    }
    case Path::Resources:
    {
        auto path_ = std::array<char, kPathBufferSize>();
        ts3Functions.getResourcesPath(path_.data(), kPathBufferSize);
        result = path_.data();
        break;
    }
    case Path::PluginIni: // honestly, just gimme a break and don't go crazy with plugin names
    {
        auto plugin_name = std::string{ts3plugin_name()};
        plugin_name.erase(std::remove_if(std::begin(plugin_name), std::end(plugin_name), ::isspace), std::end(plugin_name));
        std::transform(plugin_name.begin(), plugin_name.end(), plugin_name.begin(),
            [](unsigned char c){ return std::tolower(c); });
        plugin_name += "_plugin.ini";
        result = get_path(Path::Config) / plugin_name;
        break;
    }
    default:
        break;
    }
    return result;
}

}
