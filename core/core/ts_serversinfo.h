#pragma once

#include "ts_serverinfo_qt.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>

class TSServersInfo : public QObject
{
    Q_OBJECT

public:
	explicit TSServersInfo(QObject* parent = nullptr);
    
    TSServerInfo* get_server_info(uint64 server_connection_id, bool create_on_not_exist = false);
    uint64 find_server_by_unique_id(QString server_id);

    // forwarded from plugin.cpp
    void onConnectStatusChangeEvent(uint64 server_connection_id, int new_status, unsigned int error_number);

    void onServerGroupListEvent(uint64 server_connection_id, uint64 server_group_id, const char* name, int type, int icon_id, int save_db);
    void onServerGroupListFinishedEvent(uint64 server_connection_id);
    void onChannelGroupListEvent(uint64 server_connection_id, uint64 channel_group_id, const char* name, int type, int icon_id, int save_db);
    void onChannelGroupListFinishedEvent(uint64 server_connection_id);

signals:
    void connectStatusChanged(uint64 server_connection_id, int new_status, unsigned int error_number);

    void serverGroupListUpdated(uint64 server_connection_id, QMap<uint64,QString>);

private:
    QMap<uint64, QPointer<TSServerInfo> > m_server_infos;
};
