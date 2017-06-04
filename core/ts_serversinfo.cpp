#include "core/ts_serversinfo.h"

#include "core/ts_logging_qt.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"

TSServersInfo::TSServersInfo(QObject* parent)
	: QObject(parent)
{}

TSServerInfo* TSServersInfo::get_server_info(uint64 serverConnectionHandlerID, bool create_on_not_exist)
{
    if (m_serverInfoMap.contains(serverConnectionHandlerID))
    {
        const auto kServerInfo = m_serverInfoMap.value(serverConnectionHandlerID);
        if (!kServerInfo)
        {
            TSLogging::Error("(TSServersInfo::get_server_info): Invalid pointer.");
            return nullptr;
        }
        return kServerInfo.data();
    }
    else
    {
        if (create_on_not_exist)
        {
            auto server_info = new TSServerInfo(this, serverConnectionHandlerID);
            m_serverInfoMap.insert(serverConnectionHandlerID, server_info);
            connect(server_info, &TSServerInfo::serverGroupListUpdated, this, &TSServersInfo::serverGroupListUpdated, Qt::UniqueConnection);
            return server_info;
        }
        else
        {
            TSLogging::Error("(TSServersInfo::get_server_info): serverConnectionHandlerID not found.");
            return nullptr;
        }
    }
}

uint64 TSServersInfo::find_server_by_unique_id(QString server_id)
{
    uint64 schandler_id = 0;
    {
        uint64* servers;
        if (ts3Functions.getServerConnectionHandlerList(&servers) == ERROR_ok)
        {
            for (auto server = servers; *server; ++server)
            {
                auto ts_server_info = get_server_info(*server, true);
                if (ts_server_info && (server_id == ts_server_info->getUniqueId()))
                {
                    schandler_id = ts_server_info->getServerConnectionHandlerID();
                    break;
                }
            }
            ts3Functions.freeMemory(servers);
        }
    }
    return schandler_id;
}

void TSServersInfo::onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
    if (newStatus == STATUS_DISCONNECTED)
    {
        if (m_serverInfoMap.contains(serverConnectionHandlerID))
        {
            auto p_ServerInfo = m_serverInfoMap.value(serverConnectionHandlerID);
            if (p_ServerInfo)
                p_ServerInfo.data()->deleteLater();

            m_serverInfoMap.remove(serverConnectionHandlerID);
        }
    }

    emit connectStatusChanged(serverConnectionHandlerID, newStatus, errorNumber);
}

void TSServersInfo::onServerGroupListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, const char *name, int type, int iconID, int saveDB)
{
    auto ts_server_info = get_server_info(serverConnectionHandlerID, true);
    if (ts_server_info)
        ts_server_info->onServerGroupListEvent(serverGroupID,name,type,iconID,saveDB);
}

void TSServersInfo::onServerGroupListFinishedEvent(uint64 serverConnectionHandlerID)
{
    auto ts_server_info = get_server_info(serverConnectionHandlerID, false);
    if (ts_server_info)
        ts_server_info->onServerGroupListFinishedEvent();
}

void TSServersInfo::onChannelGroupListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, const char *name, int type, int iconID, int saveDB)
{
    auto ts_server_info = get_server_info(serverConnectionHandlerID, true);
    if (ts_server_info)
        ts_server_info->onChannelGroupListEvent(channelGroupID,name,type,iconID,saveDB);
}

void TSServersInfo::onChannelGroupListFinishedEvent(uint64 serverConnectionHandlerID)
{
    auto ts_server_info = get_server_info(serverConnectionHandlerID, false);
    if (ts_server_info)
        ts_server_info->onChannelGroupListFinishedEvent();
}
