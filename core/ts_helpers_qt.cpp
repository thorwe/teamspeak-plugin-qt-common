#include "core/ts_helpers_qt.h"

#include "teamspeak/public_rare_definitions.h"

#include "plugin.h"

#include "core/ts_functions.h"
#include "core/ts_logging_qt.h"
#include "core/ts_settings_qt.h"

#include <QtWidgets/QApplication>

#include <string>
#include <vector>

/*#ifndef RETURNCODE_BUFSIZE
#define RETURNCODE_BUFSIZE 128
#endif*/

using namespace com::teamspeak::pluginsdk;

namespace TSHelpers
{

QWidget *GetMainWindow()
{
    // Get MainWindow
    QList<QWidget *> candidates;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        if (widget->isWindow() && widget->inherits("QMainWindow") && !widget->windowTitle().isEmpty())
            candidates.append(widget);
    }

    if (candidates.count() == 1)
        return candidates.at(0);
    else
    {
        TSLogging::Error("Too many candidates for MainWindow. Report to the plugin developer.");
        return nullptr;
    }
}

std::filesystem::path PathFromQString(const QString &path)
{

#ifdef _WIN32
    auto *wptr = reinterpret_cast<const wchar_t *>(path.utf16());
    return std::filesystem::path(wptr, wptr + path.size());
#else
    return std::filesystem::path(path.toStdString());
#endif
}

QString QStringFromPath(const std::filesystem::path &path)
{
#ifdef _WIN32
    return QString::fromStdWString(path.generic_wstring());
#else
    return QString::fromStdString(path.native());
#endif
}

QString GetPath(teamspeak::plugin::Path path)
{
    const auto path_ = teamspeak::plugin::get_path(path);
    return QStringFromPath(path_);
}

QString GetFullConfigPath()
{
    auto fullPath = GetPath(teamspeak::plugin::Path::Config);
    fullPath.append(QString(ts3plugin_name()).toLower().replace(" ", ""));
    fullPath.append("_plugin.ini");
    return fullPath;
}

QString GetLanguage()
{
    QString lang;
    if (!TSSettings::instance()->GetLanguage(lang))
    {
        TSLogging::Error("(TSHelpers::GetLanguage) " + TSSettings::instance()->GetLastError().text(), true);
        return QString::null;
    }
    return lang;
}

// A normal Client-Connection (Voice-Connection) has client-type 0, a
// Query-Connection has client-type 1.
bool is_query_client(connection_id_t connection_id, client_id_t client_id)
{
    const auto [error, result] = funcs::get_client_property_as_int(connection_id, client_id, CLIENT_TYPE);
    if (ts_errc::ok != error)
        TSLogging::Error("Error checking if client is query.", connection_id, error);

    return !!result;
}

/*
std::error_code GetClientUID(connection_id_t connection_id, client_id_t client_id, QString &result)
{
    const auto [error, res] =
    funcs::get_client_property_as_string(connection_id, client_id, CLIENT_UNIQUE_IDENTIFIER);
    if (ts_errc::ok != error)
        TSLogging::Error("(TSHelpers::GetClientUID)", connection_id, error, true);
    else
        result = QString::fromUtf8(res.data());

    return error;
}*/

std::error_code
GetTalkStatus(connection_id_t connection_id, client_id_t client_id, int &status, int &is_whispering)
{
    const auto [error_talk_status, talk_status] =
    funcs::get_client_property_as_int(connection_id, client_id, CLIENT_FLAG_TALKING);
    if (ts_errc::ok != error_talk_status)
    {
        TSLogging::Error("(TSHelpers::GetTalkStatus)", connection_id, error_talk_status, true);
        return error_talk_status;
    }

    status = talk_status;
    if (status == STATUS_TALKING)
    {
        const auto [error_is_whispering, is_whispering_] = funcs::is_whispering(connection_id, client_id);
        if (ts_errc::ok != error_is_whispering)
        {
            TSLogging::Error("(TSHelpers::GetTalkStatus)", connection_id, error_is_whispering, true);
            return error_is_whispering;
        }
        is_whispering = is_whispering_;
    }
    return ts_errc::ok;
}

std::error_code GetSubChannels(connection_id_t connection_id, uint64 channel_id, std::vector<uint64> &result)
{
    const auto [error_channel_ids, channel_ids] = funcs::get_channel_ids(connection_id);
    if (ts_errc::ok != error_channel_ids)
    {
        TSLogging::Error("(TSHelpers::GetSubChannels)", connection_id, error_channel_ids, true);
        return error_channel_ids;
    }

    for (auto cid : channel_ids)
    {
        const auto [error_parent_channel_id, parent_channel_id] =
        funcs::get_parent_channel_of_channel(connection_id, cid);
        if (ts_errc::ok != error_parent_channel_id)
        {
            TSLogging::Error("(TSHelpers::GetSubChannels)", connection_id, error_parent_channel_id, true);
            return error_parent_channel_id;
        }
        if (parent_channel_id == channel_id)
            result.push_back(parent_channel_id);
    }

    return ts_errc::ok;
}

std::error_code GetServerHandler(const QString &name, connection_id_t *result)
{
    const auto [error_connection_ids, connection_ids] = funcs::get_server_connection_handler_ids();
    if (ts_errc::ok != error_connection_ids)
    {
        TSLogging::Error("(TSHelpers::GetServerHandler)getServerConnectionHandlerList", NULL,
                         error_connection_ids, true);
        return error_connection_ids;
    }

    // Find server in the list
    for (const auto connection_id : connection_ids)
    {
        const auto [error_server_name, server_name] =
        funcs::get_server_property_as_string(connection_id, VIRTUALSERVER_NAME);
        if (ts_errc::ok != error_server_name)
        {
            if (ts_errc::not_connected != error_server_name)
            {
                TSLogging::Error("(TSHelpers::GetServerHandler)getServerVariableAsString", NULL,
                                 error_server_name, true);
                return error_server_name;
            }
        }
        else
        {
            if (name == QString::fromUtf8(server_name.data()))
            {
                *result = connection_id;
                break;
            }
        }
    }

    return ts_errc::ok;
}

uint64 GetActiveServerConnectionHandlerID()
{
    // First suspect the active (mic enabled) server being the current server
    {
        const auto current_connection_id = funcs::get_current_server_connection_handler_id();
        const auto [error_input_hardware, input_hardware] =
        funcs::get_client_self_property_as_int(current_connection_id, CLIENT_INPUT_HARDWARE);
        if (ts_errc::ok != error_input_hardware)
            TSLogging::Error(
            "(TSHelpers::GetActiveServerConnectionHandlerID) Error retrieving client variable",
            current_connection_id, error_input_hardware);
        else if (input_hardware)
            return current_connection_id;
    }

    // else walk through the server list
    const auto [error_connection_ids, connection_ids] = funcs::get_server_connection_handler_ids();
    if (ts_errc::ok != error_connection_ids)
    {
        TSLogging::Error("(TSHelpers::GetActiveServerConnectionHandlerID) Error retrieving list of servers",
                         error_connection_ids);
        return 0;
    }

    // Find the first server that matches the criteria
    uint64 active = 0;
    for (const auto connection_id : connection_ids)
    {
        const auto [error_input_hardware, input_hardware] =
        funcs::get_client_self_property_as_int(connection_id, CLIENT_INPUT_HARDWARE);
        if (ts_errc::ok != error_input_hardware)
            TSLogging::Error(
            "(TSHelpers::GetActiveServerConnectionHandlerID) Error retrieving client variable", connection_id,
            error_input_hardware);
        else if (input_hardware)
        {
            active = connection_id;
            break;
        }
    }
    return active;
}

std::error_code GetActiveServerRelative(connection_id_t connection_id, bool next, uint64 *result)
{
    const auto [error_connection_ids, connection_ids] = funcs::get_server_connection_handler_ids();
    if (ts_errc::ok != error_connection_ids)
    {
        TSLogging::Error("(TSHelpers::GetActiveServerRelative) Error retrieving list of servers", NULL,
                         error_connection_ids, true);
        return error_connection_ids;
    }
    if (connection_ids.size() <= 1)
        return ts_errc::not_connected;

    auto it = std::find(std::begin(connection_ids), std::end(connection_ids), connection_id);
    if (it == std::end(connection_ids))
        return ts_errc::not_connected;

    // Find the server in the direction given
    if (next)
    {
        ++it;
        if (it == std::end(connection_ids))
            *result = connection_ids.at(0);  // Wrap around to first server
        else
            *result = *it;
    }
    else
    {
        if (it != std::begin(connection_ids))
            *result = *(--it);
        else
            *result = connection_ids.at(connection_ids.size() - 1);
    }
    return ts_errc::ok;
}

int SetActiveServer(connection_id_t connection_id)
{
    if (const auto error = funcs::sound::activate_capture_device(connection_id); ts_errc::ok != error)
    {
        TSLogging::Error("(TSHelpers::SetActiveServer) Error activating server", connection_id, error);
        return 1;
    }
    return 0;
}

int SetActiveServerRelative(connection_id_t connection_id, bool next)
{
    auto server = connection_id_t{0};
    if (const auto error = TSHelpers::GetActiveServerRelative(connection_id, next, &server);
        error != ts_errc::ok)
        return 1;

    // Check if already active
    const auto [error_input_hardware, input_hardware] =
    funcs::get_client_self_property_as_int(server, CLIENT_INPUT_HARDWARE);
    if (ts_errc::ok != error_input_hardware)
    {
        TSLogging::Error("(TSHelpers::SetActiveServerRelative) Error retrieving client variable",
                         connection_id, error_input_hardware);
        return 1;
    }
    if (!input_hardware)
        SetActiveServer(server);

    return 0;
}

namespace
{

    std::error_code GetChannelsForGroupWhisperTargetMode(connection_id_t connection_id,
                                                         client_id_t my_id,
                                                         GroupWhisperTargetMode groupWhisperTargetMode,
                                                         std::vector<uint64> &target_channels)
    {
        if (groupWhisperTargetMode != GROUPWHISPERTARGETMODE_ALL)
        {
            // get my channel
            const auto [error_my_channel, my_channel] = funcs::get_channel_of_client(connection_id, my_id);
            if (ts_errc::ok != error_my_channel)
            {
                TSLogging::Error("(TSHelpers::GetChannelsForGroupWhisperTargetMode)", connection_id,
                                 error_my_channel, true);
                return error_my_channel;
            }

            if (groupWhisperTargetMode == GROUPWHISPERTARGETMODE_CURRENTCHANNEL)
                target_channels.push_back(my_channel);
            else if (groupWhisperTargetMode == GROUPWHISPERTARGETMODE_PARENTCHANNEL)
            {
                const auto [error_parent_channel, parent_channel_id] =
                funcs::get_parent_channel_of_channel(connection_id, my_channel);
                if (ts_errc::ok != error_parent_channel)
                    return error_parent_channel;

                target_channels.push_back(parent_channel_id);
            }
            else if (groupWhisperTargetMode ==
                     GROUPWHISPERTARGETMODE_ALLPARENTCHANNELS)  // I assume it's a straight walk up the
                                                                // hierarchy?
            {
                uint64 source_channel = my_channel;
                while (true)
                {
                    const auto [error_parent_channel, parent_channel_id] =
                    funcs::get_parent_channel_of_channel(connection_id, source_channel);
                    if (ts_errc::ok != error_parent_channel)
                        return error_parent_channel;

                    if (parent_channel_id == 0)
                        break;

                    source_channel = parent_channel_id;
                    target_channels.push_back(parent_channel_id);
                }
            }
            else if (groupWhisperTargetMode ==
                     GROUPWHISPERTARGETMODE_ANCESTORCHANNELFAMILY)  // ehr, hmm, dunno, whatever
            {
            }
            else if ((groupWhisperTargetMode == GROUPWHISPERTARGETMODE_CHANNELFAMILY) ||
                     (groupWhisperTargetMode == GROUPWHISPERTARGETMODE_SUBCHANNELS))
            {
                if (const auto error = TSHelpers::GetSubChannels(connection_id, my_channel, target_channels);
                    ts_errc::ok != error)
                    return error;

                if (groupWhisperTargetMode ==
                    GROUPWHISPERTARGETMODE_CHANNELFAMILY)  // channel family: "this channel and all sub
                                                           // channels"
                    target_channels.push_back(my_channel);
            }
        }
        return ts_errc::ok;
    }
}  // namespace

std::error_code SetWhisperList(connection_id_t connection_id,
                               GroupWhisperType groupWhisperType,
                               GroupWhisperTargetMode groupWhisperTargetMode,
                               std::string_view return_code,
                               uint64 arg)
{
    const auto [error_my_id, my_id] = funcs::get_client_id(connection_id);
    if (ts_errc::ok != error_my_id)
    {
        TSLogging::Error("(TSHelpers::SetWhisperList)", connection_id, error_my_id, true);
        return error_my_id;
    }

    // GroupWhisperTargetMode
    std::vector<client_id_t> target_client_ids;
    std::vector<uint64> target_channel_ids;

    if (groupWhisperTargetMode == GROUPWHISPERTARGETMODE_ALL)  // Get client list
    {
        const auto [error_client_ids, client_ids] = funcs::get_client_ids(connection_id);
        if (ts_errc::ok != error_client_ids)
            return error_client_ids;

        for (const auto client_id : client_ids)
        {
            if (my_id != client_id)
                target_client_ids.push_back(client_id);
        }
    }
    else if (const auto error = GetChannelsForGroupWhisperTargetMode(
             connection_id, my_id, groupWhisperTargetMode, target_channel_ids);
             ts_errc::ok != error)
        return error;

    // GroupWhisperType
    if (groupWhisperType != GROUPWHISPERTYPE_ALLCLIENTS)
    {
        // We need a client list for all those, so convert channel list to clients
        if (!target_channel_ids.empty())  // same:(groupWhisperTargetMode != GROUPWHISPERTARGETMODE_ALL);
        {
            for (const auto target_channel_id : target_channel_ids)
            {
                // get clients in channel
                const auto [error_channel_client_ids, channel_client_ids] =
                funcs::get_channel_client_ids(connection_id, target_channel_id);
                if (ts_errc::ok != error_channel_client_ids)
                    return error_channel_client_ids;

                for (const auto channel_client_id : channel_client_ids)
                {
                    if (my_id != channel_client_id)
                        target_client_ids.push_back(channel_client_id);
                }
            }
            target_channel_ids.clear();
        }

        // Do the GroupWhisperType filtering
        std::vector<client_id_t> filtered_client_ids;
        if (groupWhisperType == GROUPWHISPERTYPE_SERVERGROUP)
        {
            if (arg == (uint64) NULL)
            {
                TSLogging::Error("No target server group specified. Aborting.");
                return ts_errc::parameter_invalid;
            }

            QSet<uint64> serverGroups;
            for (const auto target_client_id : target_client_ids)
            {
                if (const auto error = GetClientServerGroups(connection_id, target_client_id, &serverGroups);
                    ts_errc::ok != error)
                    return error;

                if (serverGroups.contains(arg))
                    filtered_client_ids.push_back(target_client_id);

                serverGroups.clear();
            }
        }
        else if (groupWhisperType == GROUPWHISPERTYPE_CHANNELGROUP)
        {
            // Get My Channel Group if no arg
            if (arg == (uint64) NULL)
            {
                if (const auto error = GetClientChannelGroup(connection_id, &arg); error)
                    return error;
            }

            for (const auto target_client_id : target_client_ids)
            {
                auto channelGroup = permission_group_id_t{0};
                if (const auto error = GetClientChannelGroup(connection_id, &channelGroup, target_client_id);
                    error)
                    break;

                if (channelGroup == arg)
                    filtered_client_ids.push_back(target_client_id);
            }
        }
        else if (groupWhisperType == GROUPWHISPERTYPE_CHANNELCOMMANDER)
        {
            for (const auto client_id : target_client_ids)
            {
                const auto [error_channel_commander, channel_commander] =
                funcs::get_client_property_as_int(connection_id, client_id, CLIENT_IS_CHANNEL_COMMANDER);
                if (ts_errc::ok != error_channel_commander)
                    continue;

                if (channel_commander == 1)
                    filtered_client_ids.push_back(client_id);
            }
        }

        target_client_ids = filtered_client_ids;
    }

    if (target_channel_ids.empty() && target_client_ids.empty())
        return ts_errc::ok_no_update;
    else
    {
        TSLogging::Log("Attempting to whisper to:", connection_id, LogLevel_DEBUG);

        if (!target_channel_ids.empty())
        {
            QString log_str = "ChannelIds: ";
            for (const auto target_channel_id : target_channel_ids)
            {
                log_str.append(QString("%1").arg(target_channel_id));
                log_str.append(" ");
            }

            TSLogging::Log(log_str.toLocal8Bit().constData(), connection_id, LogLevel_DEBUG);
        }

        if (!target_client_ids.empty())
        {
            QString log_str = "Clients: ";
            for (auto target_client_id : target_client_ids)
            {
                const auto [error_display_name, display_name] =
                funcs::get_client_display_name(connection_id, target_client_id);
                if (ts_errc::ok != error_display_name)
                {
                    log_str.append("(Error getting client display name)");
                }
                else
                    log_str.append(QString::fromUtf8(display_name.data()));

                log_str.append(" ");
            }
            TSLogging::Log(log_str.toLocal8Bit().constData(), connection_id, LogLevel_DEBUG);
        }

        const auto error = funcs::request_client_set_whisperlist(connection_id, my_id, target_channel_ids,
                                                                 target_client_ids, return_code);

        if (error || (return_code.empty()))
            return error;

        // Bandaid for requestClientSetWhisperList NOT calling back when return_code set. For waffle's sake!
        return funcs::request_client_properties(connection_id, my_id, return_code.data());
    }
}

std::error_code GetDefaultProfile(PluginGuiProfile profile, QString &result)
{
    const auto [error, default_profile, profiles] = funcs::get_profile_list(profile);
    if (ts_errc::ok != error)
        TSLogging::Error("Error retrieving profiles", error);
    else
        result = QString::fromUtf8(profiles[default_profile].data());

    return error;
}

std::error_code
GetClientServerGroups(connection_id_t connection_id, client_id_t client_id, QSet<uint64> *result)
{
    const auto [error, server_groups_s] =
    funcs::get_client_property_as_string(connection_id, client_id, CLIENT_SERVERGROUPS);
    if (ts_errc::ok == error)
    {
        auto qsl_result = QString::fromStdString(server_groups_s).split(",", QString::SkipEmptyParts);
        bool ok;
        while (!qsl_result.isEmpty())
        {
            *result << qsl_result.takeFirst().toInt(&ok);
            if (!ok)
            {
                TSLogging::Error("Error converting Server Group to int");
                return ts_errc::not_implemented;
            }
        }
    }
    return error;
}

std::error_code GetClientSelfServerGroups(connection_id_t connection_id, QSet<uint64> *result)
{
    const auto [error_my_id, my_id] = funcs::get_client_id(connection_id);
    if (ts_errc::ok != error_my_id)
    {
        TSLogging::Error("(TSHelpers::GetClientSelfServerGroups)", connection_id, error_my_id, true);
        return error_my_id;
    }

    return GetClientServerGroups(connection_id, my_id, result);
}

std::error_code GetClientChannelGroup(connection_id_t connection_id, uint64 *result, client_id_t client_id)
{
    if (0 == client_id)  // use my id
    {
        const auto [error_my_id, my_id] = funcs::get_client_id(connection_id);
        if (ts_errc::ok != error_my_id)
        {
            TSLogging::Error("(GetClientChannelGroup)", connection_id, error_my_id, true);
            return error_my_id;
        }
    }

    const auto [error_channel_group_id, channel_group_id] =
    funcs::get_client_property_as_int(connection_id, client_id, CLIENT_CHANNEL_GROUP_ID);
    if (ts_errc::ok != error_channel_group_id)
        TSLogging::Error("(GetClientChannelGroup)", connection_id, error_channel_group_id, true);
    else
        *result = (uint64) channel_group_id;

    return error_channel_group_id;
}

bool GetCreatePluginConfigFolder(QDir &result)
{
    auto isPathOk = true;
    auto path = TSHelpers::GetPath(teamspeak::plugin::Path::Config);
    QDir dir(path);
    if (!dir.exists())
    {
        TSLogging::Error("(GetCreatePluginConfigFolder) Config Path does not exist.", true);
        isPathOk = false;
    }
    else
    {
        path = QString(ts3plugin_name()).toLower();
        if (!dir.exists(path))
        {
            if (!dir.mkdir(path))
            {
                TSLogging::Error("(GetCreatePluginConfigFolder) Could not create cache folder.", true);
                isPathOk = false;
            }
        }
        if (isPathOk && !dir.cd(path))
        {
            TSLogging::Error("(GetCreatePluginConfigFolder) Could not enter cache folder.");
            isPathOk = false;
        }
    }
    if (isPathOk)
        result = dir;

    return isPathOk;
}

QString GetChannelVariableAsQString(connection_id_t connection_id,
                                    uint64 channel_id,
                                    ChannelProperties channel_property)
{
    const auto [error, result] =
    funcs::get_channel_property_as_string(connection_id, channel_id, channel_property);
    if (ts_errc::ok != error)
    {
        TSLogging::Error("Error on getChannelVariableAsString", connection_id, error);
        return QString::null;
    }
    return QString::fromUtf8(result.c_str());
}

// Note that we have the convention of the delimiter "__CH_DELIM__" here and with GetChannelIDFromPath
QString GetChannelPath(connection_id_t connection_id, uint64 channel_id)
{
    QString path = QString::null;
    while (true)
    {
        auto name = GetChannelVariableAsQString(connection_id, channel_id, CHANNEL_NAME);
        path.prepend(name);

        const auto [error_parent_channel_id, parent_channel_id] =
        funcs::get_parent_channel_of_channel(connection_id, channel_id);
        if (ts_errc::ok != error_parent_channel_id)
        {
            TSLogging::Error("(GetChannelPath) Error getting channel id from channel names.", connection_id,
                             error_parent_channel_id);
            return QString::null;
        }
        if (!parent_channel_id)
            return path;

        channel_id = parent_channel_id;
        path.prepend("__CH_DELIM__");
    }
}

// Note that we have the convention of the delimiter "__CH_DELIM__" here and with GetChannelPath
uint64 GetChannelIDFromPath(connection_id_t connection_id, const QString &path_q)
{
    std::vector<std::string> channel_names;
    const auto split_list = path_q.split("__CH_DELIM__");
    for (const auto &channel_name : split_list)
        channel_names.emplace_back(channel_name.toUtf8().constData());

    const auto [error, channel_id] = funcs::get_channel_id_from_channel_names(connection_id, channel_names);
    if (ts_errc::ok != error)
    {
        TSLogging::Error("(GetChannelIDFromPath) Error getting channel id from channel names.", connection_id,
                         error);
        return 0;
    }

    return channel_id;
}

}  // namespace TSHelpers
