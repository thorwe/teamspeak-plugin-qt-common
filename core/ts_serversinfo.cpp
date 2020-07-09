#include "core/ts_serversinfo.h"

#include "core/ts_functions.h"
#include "core/ts_logging_qt.h"

#include "plugin.h"

#include "teamspeak/clientlib_publicdefinitions.h"

using namespace com::teamspeak::pluginsdk;

TSServersInfo::TSServersInfo(QObject* parent)
	: QObject(parent)
{}

TSServerInfo* TSServersInfo::get_server_info(uint64 server_connection_id, bool create_on_not_exist)
{
    if (m_server_infos.contains(server_connection_id))
    {
        const auto kServerInfo = m_server_infos.value(server_connection_id);
        if (!kServerInfo)
        {
            TSLogging::Error("(TSServersInfo::get_server_info): Invalid pointer.");
            return nullptr;
        }
        return kServerInfo.data();
    }

    if (create_on_not_exist)
    {
        auto server_info = new TSServerInfo(this, server_connection_id);
        m_server_infos.insert(server_connection_id, server_info);
        connect(server_info, &TSServerInfo::serverGroupListUpdated, this, &TSServersInfo::serverGroupListUpdated, Qt::UniqueConnection);
        return server_info;
    }

    TSLogging::Error("(TSServersInfo::get_server_info): server_connection_id not found.");
    return nullptr;
}

uint64 TSServersInfo::find_server_by_unique_id(const QString &server_uid)
{
    uint64 server_connection_id = 0;
    {
        const auto [error_connection_ids, connection_ids] = funcs::get_server_connection_handler_ids();
        if (ts_errc::ok == error_connection_ids)
        {
            for (auto connection_id : connection_ids)
            {
                auto ts_server_info = get_server_info(connection_id, true);
                if (ts_server_info && (server_uid == ts_server_info->getUniqueId()))
                {
                    server_connection_id = connection_id;
                    break;
                }
            }
        }
    }
    return server_connection_id;
}

void TSServersInfo::onConnectStatusChangeEvent(uint64 server_connection_id, int new_status, unsigned int error_number)
{
    if (new_status == STATUS_DISCONNECTED)
    {
        if (m_server_infos.contains(server_connection_id))
        {
            auto p_ServerInfo = m_server_infos.value(server_connection_id);
            if (p_ServerInfo)
                p_ServerInfo.data()->deleteLater();

            m_server_infos.remove(server_connection_id);
        }
    }

    emit connectStatusChanged(server_connection_id, new_status, error_number);
}

void TSServersInfo::onServerGroupListEvent(uint64 server_connection_id, uint64 server_group_id, const char *name, int type, int icon_id, int save_db)
{
    auto ts_server_info = get_server_info(server_connection_id, true);
    if (ts_server_info)
        ts_server_info->onServerGroupListEvent(server_group_id, name, type, icon_id, save_db);
}

void TSServersInfo::onServerGroupListFinishedEvent(uint64 server_connection_id)
{
    auto ts_server_info = get_server_info(server_connection_id, false);
    if (ts_server_info)
        ts_server_info->onServerGroupListFinishedEvent();
}

void TSServersInfo::onChannelGroupListEvent(uint64 server_connection_id, uint64 channel_group_id, const char *name, int type, int icon_id, int save_db)
{
    auto ts_server_info = get_server_info(server_connection_id, true);
    if (ts_server_info)
        ts_server_info->onChannelGroupListEvent(channel_group_id, name, type, icon_id, save_db);
}

void TSServersInfo::onChannelGroupListFinishedEvent(uint64 server_connection_id)
{
    auto ts_server_info = get_server_info(server_connection_id, false);
    if (ts_server_info)
        ts_server_info->onChannelGroupListFinishedEvent();
}
