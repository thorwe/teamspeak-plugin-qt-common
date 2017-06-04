#pragma once

#include "ts_serverinfo_qt.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>

class TSServersInfo : public QObject
{
    Q_OBJECT

public:
	explicit TSServersInfo(QObject* parent = nullptr);
    
    TSServerInfo* get_server_info(uint64 serverConnectionHandlerID, bool create_on_not_exist = false);
    uint64 find_server_by_unique_id(QString server_id);

    // forwarded from plugin.cpp
    void onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

    void onServerGroupListEvent(uint64 serverConnectionHandlerID, uint64 serverGroupID, const char* name, int type, int iconID, int saveDB);
    void onServerGroupListFinishedEvent(uint64 serverConnectionHandlerID);
    void onChannelGroupListEvent(uint64 serverConnectionHandlerID, uint64 channelGroupID, const char* name, int type, int iconID, int saveDB);
    void onChannelGroupListFinishedEvent(uint64 serverConnectionHandlerID);

signals:
    void connectStatusChanged(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

    void serverGroupListUpdated(uint64 serverConnectionHandlerID, QMap<uint64,QString>);

private:
    QMap<uint64, QPointer<TSServerInfo> > m_serverInfoMap;
};
