#pragma once

#include <atomic>

#include "teamspeak/public_definitions.h"

#include "ts_servergroups.h"

#include <QtCore/QObject>

class TSServerInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint64 serverConnectionHandlerID READ getServerConnectionHandlerID)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QString uniqueId READ getUniqueId)
    Q_PROPERTY(uint64 defaultChannelGroup READ getDefaultChannelGroup)

public:
    explicit TSServerInfo(QObject *parent = 0, uint64 server_connection_id = 0);

    uint64 getServerConnectionHandlerID() const;
    
	QString getName() const;
    QString getUniqueId() const;
    uint64 getDefaultChannelGroup() const;

    // forwards from plugin.cpp
    void onServerGroupListEvent(uint64 server_group_id, const char* name, int type, int icon_id, int save_db);
    void onServerGroupListFinishedEvent();
    void onChannelGroupListEvent(uint64 channel_group_id, const char* name, int type, int icon_id, int save_db);
    void onChannelGroupListFinishedEvent();

    uint64 GetServerGroupId(QString name) const;
    QString GetServerGroupName(uint64 id) const;
    uint64 GetChannelGroupId(QString name) const;
    QString GetChannelGroupName(uint64 id) const;

signals:
    void serverGroupListUpdated(uint64 connection_id, QMap<uint64, QString>);
    void channelGroupListUpdated(uint64 connection_id, QMap<uint64, QString>);

private:
    uint64 m_server_connection_id;

    QMap<uint64, QString> m_server_groups;
    bool m_is_server_groups_updating = false;
    QMap<uint64, QString> m_channel_groups;
    bool m_is_channel_groups_updating = false;
};
