#include "core/talkers.h"

#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_rare_definitions.h"
#include "ts3_functions.h"

#include "core/ts_logging_qt.h"

#include "plugin.h"

Talkers::Talkers(QObject* parent)
	: QObject(parent)
{}

unsigned int Talkers::RefreshTalkers(uint64 serverConnectionHandlerID)
{
    unsigned int error = ERROR_ok;
    int status;
    if ((error = ts3Functions.getConnectionStatus(serverConnectionHandlerID, &status)) != ERROR_ok)
        return error;

    if (status != STATUS_CONNECTION_ESTABLISHED)
        return ERROR_ok;

    anyID myID;
    if ((error = ts3Functions.getClientID(serverConnectionHandlerID, &myID)) != ERROR_ok)
        return error;

    int talking;
    if ((error = ts3Functions.getClientSelfVariableAsInt(serverConnectionHandlerID, CLIENT_FLAG_TALKING, &talking)) != ERROR_ok)
        return error;

    if (talking == STATUS_TALKING)
    {
        int isWhispering;
        if ((error = ts3Functions.isWhispering(serverConnectionHandlerID, myID, &isWhispering)) != ERROR_ok)
            return error;

        onTalkStatusChangeEvent(serverConnectionHandlerID, STATUS_TALKING, isWhispering, myID);
    }

    //Get all visible clients
    anyID *clientList;
    if ((error = ts3Functions.getClientList(serverConnectionHandlerID, &clientList)) != ERROR_ok)
        return error;

    for (int i=0; clientList[i]; ++i)
    {
        const auto kClientId = clientList[i];
        if (ts3Functions.getClientVariableAsInt(serverConnectionHandlerID, kClientId, CLIENT_FLAG_TALKING, &talking) == ERROR_ok)
        {
            if (talking == STATUS_TALKING)
            {
                int isWhispering;
                if ((error = ts3Functions.isWhispering(serverConnectionHandlerID, kClientId, &isWhispering)) != ERROR_ok)
                    continue;

                onTalkStatusChangeEvent(serverConnectionHandlerID, STATUS_TALKING, isWhispering, kClientId);
            }
        }
    }
    ts3Functions.freeMemory(clientList);
    return error;
}

unsigned int Talkers::RefreshAllTalkers()  // I assume getClientVariableAsInt only returns whisperer to me as talking and isWhispering == isWhisperingMe
{
    unsigned int error = ERROR_ok;
    uint64* serverList;
    if(ts3Functions.getServerConnectionHandlerList(&serverList) == ERROR_ok)
    {
        for (int i=0; serverList[i] != NULL; ++i)
        {
            if ((error = RefreshTalkers(serverList[i])) != ERROR_ok)
                break;
        }
        ts3Functions.freeMemory(serverList);
    }
    return error;
}

void Talkers::DumpTalkStatusChanges(QObject *p, int status)
{
    auto iTalk = qobject_cast<TalkInterface*>(p);
    if (!iTalk)
    {
        TSLogging::Error("(Talkers) (DumpTalkStatusChange) Pointer doesn't implement TalkInterface");
        return;
    }

    for (auto it = TalkerMap.cbegin(); it != TalkerMap.cend(); ++it)
        iTalk->onTalkStatusChanged(it.key(), status, false, it.value(), false);

    for (auto it = WhisperMap.cbegin(); it != WhisperMap.cend(); ++it)
        iTalk->onTalkStatusChanged(it.key(), status, true, it.value(), false);

    if (m_meTalkingScHandler != 0)
    {
        unsigned int error;
        // Get My Id on this handler
        anyID myID;
        if ((error = ts3Functions.getClientID(m_meTalkingScHandler,&myID)) != ERROR_ok)
        {
            TSLogging::Error("DumpTalkStatusChanges", m_meTalkingScHandler, error);
            return;
        }
        iTalk->onTalkStatusChanged(m_meTalkingScHandler, status, m_meTalkingIsWhisper, myID, true);
    }
}

bool Talkers::onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID)
{
    unsigned int error;

    // Get My Id on this handler
    anyID myID;
    if ((error = ts3Functions.getClientID(serverConnectionHandlerID, &myID)) != ERROR_ok)
    {
        TSLogging::Error("onTalkStatusChangeEvent", serverConnectionHandlerID, error);
        return false;
    }

    if (clientID == myID)
    {
        m_meTalkingIsWhisper = isReceivedWhisper;
        if (status == STATUS_TALKING)
            m_meTalkingScHandler = serverConnectionHandlerID;
        else
            m_meTalkingScHandler = 0;

        return true;
    }

    if (status == STATUS_TALKING)
    {
        if (isReceivedWhisper)
        {
            if (!WhisperMap.contains(serverConnectionHandlerID,clientID))   // pure safety measurement
                WhisperMap.insert(serverConnectionHandlerID,clientID);
        }
        else
        {
            if (!TalkerMap.contains(serverConnectionHandlerID,clientID))
                TalkerMap.insert(serverConnectionHandlerID,clientID);
        }
    }
    else if (status == STATUS_NOT_TALKING)
    {
        if (isReceivedWhisper)
            WhisperMap.remove(serverConnectionHandlerID,clientID);
        else
            TalkerMap.remove(serverConnectionHandlerID,clientID);
    }

    return false;
}

void Talkers::onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber)
{
    if (newStatus == STATUS_DISCONNECTED)
    {
        if (WhisperMap.contains(serverConnectionHandlerID))
        {
            for (const auto& value : WhisperMap.values(serverConnectionHandlerID))
                ts3plugin_onTalkStatusChangeEvent(serverConnectionHandlerID, STATUS_NOT_TALKING, 0, value);
        }

        if (TalkerMap.contains(serverConnectionHandlerID))
        {
            for (const auto& value : TalkerMap.values(serverConnectionHandlerID))
                ts3plugin_onTalkStatusChangeEvent(serverConnectionHandlerID, STATUS_NOT_TALKING, 0, value);
        }
    }
    emit ConnectStatusChanged(serverConnectionHandlerID, newStatus, errorNumber);
}

const QMultiMap<uint64, anyID>& Talkers::GetTalkerMap() const
{
    return TalkerMap;
}

const QMultiMap<uint64, anyID>& Talkers::GetWhisperMap() const
{
    return WhisperMap;
}

uint64 Talkers::isMeTalking() const
{
    return m_meTalkingScHandler;
}
