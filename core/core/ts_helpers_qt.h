#pragma once

#include "core/definitions.h"
#include "core/plugin_helpers.h"

#include "plugin_definitions.h"
#include "teamspeak/public_definitions.h"

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtWidgets/QWidget>

#include <filesystem>
#include <string_view>
#include <system_error>
#include <vector>

namespace TSHelpers
{
using namespace com::teamspeak;

QWidget *GetMainWindow();

std::filesystem::path PathFromQString(const QString &path);
QString QStringFromPath(const std::filesystem::path &path);

QString GetPath(teamspeak::plugin::Path);

QString GetLanguage();

bool is_query_client(connection_id_t connection_id, client_id_t client_id);

std::error_code
GetTalkStatus(connection_id_t connection_id, client_id_t client_id, int &status, int &isWhispering);
std::error_code GetSubChannels(connection_id_t connection_id, uint64 channelId, std::vector<uint64> &result);

/* credits @ Jules Blok (jules@aerix.nl),
 * see
 * https://github.com/Armada651/g-key
 * for the original std approach
 */
std::error_code GetServerHandler(const QString &name, connection_id_t *result);
uint64 GetActiveServerConnectionHandlerID();
std::error_code GetActiveServerRelative(connection_id_t connection_id, bool next, uint64 *result);
int SetActiveServer(connection_id_t connection_id);
int SetActiveServerRelative(connection_id_t connection_id, bool next);
inline int SetNextActiveServer(connection_id_t connection_id)
{
    return SetActiveServerRelative(connection_id, true);
}
inline int SetPrevActiveServer(connection_id_t connection_id)
{
    return SetActiveServerRelative(connection_id, false);
}
std::error_code SetWhisperList(connection_id_t connection_id,
                               GroupWhisperType groupWhisperType,
                               GroupWhisperTargetMode groupWhisperTargetMode,
                               std::string_view return_code = "",
                               uint64 arg = 0);

std::error_code GetDefaultProfile(PluginGuiProfile profile, QString &result);

std::error_code
GetClientServerGroups(connection_id_t connection_id, client_id_t client_id, QSet<uint64> *result);
std::error_code GetClientSelfServerGroups(connection_id_t connection_id, QSet<uint64> *result);
std::error_code
GetClientChannelGroup(connection_id_t connection_id, uint64 *result, client_id_t client_id = 0);

bool GetCreatePluginConfigFolder(QDir &result);

QString
GetChannelVariableAsQString(connection_id_t connection_id, uint64 channelID, ChannelProperties property);
QString GetChannelPath(connection_id_t connection_id, uint64 channel_id);
uint64 GetChannelIDFromPath(connection_id_t connection_id, const QString &path_q);
}  // namespace TSHelpers
