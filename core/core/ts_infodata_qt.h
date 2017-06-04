#pragma once

#include <QtCore/QObject>
#include <QtCore/QTextStream>
#include <QtCore/QMutex>

#include "teamspeak/public_definitions.h"
#include "plugin_definitions.h"

class InfoDataInterface
{
public:
    virtual bool onInfoDataChanged(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, uint64 mine, QTextStream &data) = 0;
};
Q_DECLARE_INTERFACE(InfoDataInterface, "com.teamspeak.InfoDataInterface/1.0")

class TSInfoData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PluginItemType infoType
               READ getInfoType
               NOTIFY infoTypeChanged)
    Q_PROPERTY(uint64 infoId
               READ getInfoId
               NOTIFY infoIdChanged)

public:
	TSInfoData(QObject *parent = nullptr);
	~TSInfoData() = default;

    PluginItemType getInfoType() const;
    uint64 getInfoId() const;

//    void setHomeId(uint64 serverConnectionHandlerID);
    
    bool Register(QObject *p, bool isRegister, int priority);
    void onInfoData(uint64 serverConnectionHandlerID, uint64 id, PluginItemType type, char **data);

signals:
    void infoTypeChanged(PluginItemType);
    void infoIdChanged(uint64);
    
public slots:
    void RequestUpdate(uint64 serverConnectionHandlerID, uint64 id, PluginItemType type);
    void RequestUpdate(uint64 serverConnectionHandlerID, uint64 id) {RequestUpdate(serverConnectionHandlerID,id,PLUGIN_CHANNEL);}
    void RequestUpdate(uint64 serverConnectionHandlerID, anyID id) {RequestUpdate(serverConnectionHandlerID,id,PLUGIN_CLIENT);}
    void RequestUpdate(uint64 serverConnectionHandlerID) {RequestUpdate(serverConnectionHandlerID,NULL,PLUGIN_SERVER);}
    void RequestSelfUpdate();

private:
    PluginItemType m_InfoType = PLUGIN_SERVER;
    uint64 m_InfoId = 0;

    uint64 m_homeId = 0;

    QMultiMap<int, QPointer<QObject> > m_callbacks;
};
