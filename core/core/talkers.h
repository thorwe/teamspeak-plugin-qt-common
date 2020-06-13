#pragma once

#include "module.h"

#include "teamspeak/public_definitions.h"

#include <QtCore/QMultiMap>
#include <QtCore/QObject>

#include <unordered_map>

class TalkInterface
{
public:
    virtual bool onTalkStatusChanged(uint64 serverConnectionHandlerID, int status, bool isReceivedWhisper, anyID clientID, bool itsMe) = 0;
};
Q_DECLARE_INTERFACE(TalkInterface,"com.teamspeak.TalkInterface/1.0")

class Talkers : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint64 isMeTalking
               READ isMeTalking)

public:
	Talkers(QObject* parent = nullptr);

    bool onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID);
    void onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

    const QMultiMap<uint64, anyID>& GetTalkerMap() const;
    const QMultiMap<uint64, anyID>& GetWhisperMap() const;
    uint64 isMeTalking() const;

    std::error_code RefreshTalkers(uint64 connection_id);
    std::error_code RefreshAllTalkers();

    void DumpTalkStatusChanges(QObject* p, int status);

signals:
    void ConnectStatusChanged(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

private:
    uint64 m_meTalkingScHandler = 0;
    bool m_meTalkingIsWhisper{ false };

    QMultiMap<uint64, anyID> TalkerMap;
    QMultiMap<uint64, anyID> WhisperMap;
};
