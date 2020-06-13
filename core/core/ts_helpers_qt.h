#pragma once

#include "core/plugin_helpers.h"

#include "teamspeak/public_definitions.h"
#include "plugin_definitions.h"

#include <QtCore/QString>
#include <QtWidgets/QWidget>
#include <QtCore/QDir>

#include <filesystem>
#include <system_error>
#include <vector>

namespace TSHelpers
{
QWidget *GetMainWindow();

std::filesystem::path PathFromQString(const QString &path);
QString QStringFromPath(const std::filesystem::path &path);

QString GetPath(teamspeak::plugin::Path);

QString GetLanguage();

bool is_query_client(uint64 connection_id, anyID client_id);

/*
std::error_code GetClientUID(uint64 serverConnectionHandlerID, anyID clientID, QString &result);
std::error_code
GetTalkStatus(uint64 serverConnectionHandlerID, anyID clientID, int &status, int &isWhispering);
std::error_code
GetSubChannels(uint64 serverConnectionHandlerID, uint64 channelId, std::vector<uint64> &result);*/

/* credits @ Jules Blok (jules@aerix.nl),
 * see
 * https://github.com/Armada651/g-key
 * for the original std approach
 */
/*std::error_code GetServerHandler(QString name, uint64 *result);
uint64 GetActiveServerConnectionHandlerID();
std::error_code GetActiveServerRelative(uint64 serverConnectionHandlerID, bool next, uint64 *result);
int SetActiveServer(uint64 serverConnectionHandlerID);
int SetActiveServerRelative(uint64 serverConnectionHandlerID, bool next);
inline int SetNextActiveServer(uint64 serverConnectionHandlerID) { return
SetActiveServerRelative(serverConnectionHandlerID, true); } inline int SetPrevActiveServer(uint64
serverConnectionHandlerID) { return SetActiveServerRelative(serverConnectionHandlerID, false); }
std::error_code SetWhisperList(uint64 serverConnectionHandlerID,
                               GroupWhisperType groupWhisperType,
                               GroupWhisperTargetMode groupWhisperTargetMode,
                               QString returnCode = QString::null,
                               uint64 arg = (uint64) NULL);

std::error_code GetDefaultProfile(PluginGuiProfile profile, QString &result);

std::error_code
GetClientServerGroups(uint64 serverConnectionHandlerID, anyID clientID, QSet<uint64> *result);
std::error_code GetClientSelfServerGroups(uint64 serverConnectionHandlerID, QSet<uint64> *result);
std::error_code
GetClientChannelGroup(uint64 serverConnectionHandlerID, uint64 *result, anyID clientId = (anyID) NULL);

bool GetCreatePluginConfigFolder(QDir &result);

QString GetChannelVariableAsQString(uint64 serverConnectionHandlerID, uint64 channelID, ChannelProperties
property); QString GetChannelPath(uint64 serverConnectionHandlerID, uint64 channel_id); uint64
GetChannelIDFromPath(uint64 serverConnectionHandlerID, QString path_q);*/
}  // namespace TSHelpers
