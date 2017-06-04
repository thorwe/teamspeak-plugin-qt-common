#pragma once

#include <QtCore/QObject>
#include <QtCore/QMultiMap>

#include "teamspeak/public_definitions.h"

#include <unordered_map>

#include "module.h"

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

    unsigned int RefreshTalkers(uint64 serverConnectionHandlerID);
    unsigned int RefreshAllTalkers();

    void DumpTalkStatusChanges(QObject* p, int status);

signals:
    void ConnectStatusChanged(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

private:
    uint64 m_meTalkingScHandler = 0;
    bool m_meTalkingIsWhisper;

    QMultiMap<uint64, anyID> TalkerMap;
    QMultiMap<uint64, anyID> WhisperMap;
};
