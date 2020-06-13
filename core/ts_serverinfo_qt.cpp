#include "core/ts_serverinfo_qt.h"

#include "teamspeak/public_rare_definitions.h"

#include "ts_missing_definitions.h"

#include "core/ts_functions.h"
#include "core/ts_helpers_qt.h"
#include "core/ts_logging_qt.h"
#include "plugin.h"

using namespace com::teamspeak::pluginsdk;

TSServerInfo::TSServerInfo(QObject *parent, uint64 server_connection_id)
	: QObject(parent)
	, m_server_connection_id(server_connection_id)
{}

uint64 TSServerInfo::getServerConnectionHandlerID() const
{
    return m_server_connection_id;
}

QString TSServerInfo::getName() const
{
    const auto [error_server_name, server_name] = funcs::get_server_property_as_string(m_server_connection_id, VIRTUALSERVER_NAME);
    if (ts_errc::ok != error_server_name)
    {
        TSLogging::Error("(TSServerInfo::getName())", NULL, error_server_name, true);
        return QString::null;
    }

    return QString::fromUtf8(server_name.data());
}

QString TSServerInfo::getUniqueId() const
{
    const auto [error_server_uid, server_uid] = funcs::get_server_property_as_string(m_server_connection_id, VIRTUALSERVER_UNIQUE_IDENTIFIER);
    if (ts_errc::ok != error_server_uid)
    {
        TSLogging::Error("(TSServerInfo::getUniqueId())", NULL, error_server_uid, true);
        return QString::null;
    }
    return QString::fromUtf8(server_uid.data());
}

uint64 TSServerInfo::getDefaultChannelGroup() const
{
    const auto [error_default_channel_group, default_channel_group] = funcs::get_server_property_as_uint64(m_server_connection_id, VIRTUALSERVER_DEFAULT_CHANNEL_GROUP);
    if (ts_errc::ok != error_default_channel_group)
    {
        TSLogging::Error("Could not get default channel group", m_server_connection_id, error_default_channel_group, true);
        return 0;
    }
    return default_channel_group;
}

uint64 TSServerInfo::GetServerGroupId(QString name) const
{
    return (m_server_groups.key(name, (uint64)NULL));
}

QString TSServerInfo::GetServerGroupName(uint64 id) const
{
    if (m_server_groups.contains(id))
        return m_server_groups.value(id);

    return QString::null;
}

uint64 TSServerInfo::GetChannelGroupId(QString name) const
{
    const auto list = m_channel_groups.keys(name);
    const auto element_count = list.size();
    if (element_count == 0)
        return 0;

    if (element_count == 1)
        return list.at(0);

    uint64 highest = 0;
    for (int i = 0; i < element_count; ++i)
    {
        uint64 e_key = list.at(i);
        if (e_key > highest)
            highest = e_key;
    }
    return highest;
}

QString TSServerInfo::GetChannelGroupName(uint64 id) const
{
    if (m_channel_groups.contains(id))
        return m_channel_groups.value(id);

    return QString::null;
}

void TSServerInfo::onServerGroupListEvent(uint64 server_group_id, const char *name, int type, int icon_id, int save_db)
{
    Q_UNUSED(type);
    Q_UNUSED(icon_id);
    Q_UNUSED(save_db);

    if (!m_is_server_groups_updating)
        m_server_groups.clear();

    m_server_groups.insert(server_group_id, name);
    m_is_server_groups_updating = true;
}

void TSServerInfo::onServerGroupListFinishedEvent()
{
    m_is_server_groups_updating = false;
    emit serverGroupListUpdated(m_server_connection_id, m_server_groups);
}

void TSServerInfo::onChannelGroupListEvent(uint64 channel_group_id, const char *name, int type, int icon_id, int save_db)
{
    if (type != PermGroupDBTypeRegular) // For now discard templates and Server Queries
        return;

    Q_UNUSED(icon_id);
    Q_UNUSED(save_db);

    if (!m_is_channel_groups_updating)
        m_channel_groups.clear();

    m_channel_groups.insert(channel_group_id, name);
    m_is_channel_groups_updating = true;
}

void TSServerInfo::onChannelGroupListFinishedEvent()
{
    m_is_channel_groups_updating = false;
    emit channelGroupListUpdated(m_server_connection_id, m_channel_groups);
}
