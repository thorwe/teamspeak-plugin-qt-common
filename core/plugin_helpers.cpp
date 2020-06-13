#include "core/plugin_helpers.h"

#include "core/ts_functions.h"

#include "plugin.h"

#include <algorithm>
#include <cctype>
#include <string>

using namespace com::teamspeak::pluginsdk;

namespace teamspeak::plugin
{

std::filesystem::path get_path(Path path)
{
    std::filesystem::path result;

    switch (path)
    {
    case Path::App:
    {
        result = funcs::get_app_path().data();
        break;
    }
    case Path::Config:
    {
        result = funcs::get_config_path().data();
        break;
    }
    case Path::Resources:
    {
        result = funcs::get_resources_path().data();
        break;
    }
    case Path::PluginIni: // honestly, just gimme a break and don't go crazy with plugin names or do a pull request
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

} // namespace teamspeak::plugin
