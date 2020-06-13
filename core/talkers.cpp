#include "core/talkers.h"

#include "core/ts_functions.h"

#include "teamspeak/public_rare_definitions.h"

#include "core/ts_logging_qt.h"

#include "plugin.h"

using namespace com::teamspeak::pluginsdk;

Talkers::Talkers(QObject* parent)
	: QObject(parent)
{}

std::error_code Talkers::RefreshTalkers(uint64 connection_id)
{
    const auto[error_connection_status, connection_status] = funcs::get_connection_status(connection_id);
    if (ts_errc::ok != error_connection_status)
        return error_connection_status;

    if (connection_status != STATUS_CONNECTION_ESTABLISHED)
        return ts_errc::ok;

    const auto[error_my_id, my_id] = funcs::get_client_id(connection_id);
    if (ts_errc::ok != error_my_id)
        return error_my_id;

    const auto[error_my_talking_flag, my_talking_flag] = funcs::get_client_self_property_as_int(connection_id, CLIENT_FLAG_TALKING);
    if (ts_errc::ok != error_my_talking_flag)
        return error_my_talking_flag;


    if (my_talking_flag == STATUS_TALKING)
    {
        const auto [error_is_whispering, is_whispering] = funcs::is_whispering(connection_id, my_id);
        if (ts_errc::ok != error_is_whispering)
            return error_is_whispering;

        onTalkStatusChangeEvent(connection_id, STATUS_TALKING, is_whispering, my_id);
    }

    //Get all visible clients
    const auto[error_client_ids, client_ids] = funcs::get_client_ids(connection_id);
    if (ts_errc::ok != error_client_ids)
        return error_client_ids;

    for (const auto client_id : client_ids)
    {
        const auto [error_flag_talking, flag_talking] = funcs::get_client_property_as_int(connection_id, client_id, CLIENT_FLAG_TALKING);
        if (ts_errc::ok != error_flag_talking)
            continue;

        if (STATUS_TALKING == flag_talking)
        {
            const auto [error_is_whispering, is_whispering] = funcs::is_whispering(connection_id, client_id);
            if (ts_errc::ok != error_is_whispering)
                continue;

            onTalkStatusChangeEvent(connection_id, STATUS_TALKING, is_whispering, client_id);
        }
    }

    return ts_errc::ok;
}

std::error_code Talkers::RefreshAllTalkers()  // I assume getClientVariableAsInt only returns whisperer to me
                                              // as talking and isWhispering == isWhisperingMe
{
    const auto[error_connection_ids, connection_ids] = funcs::get_server_connection_handler_ids();
    if (ts_errc::ok != error_connection_ids)
        return error_connection_ids;

    for (const auto connection_id : connection_ids)
    {
        if (const auto error = RefreshTalkers(connection_id); ts_errc::ok != error)
            return error;
    }
    return ts_errc::ok;
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
        const auto [error_my_id, my_id] = funcs::get_client_id(m_meTalkingScHandler);
        if (ts_errc::ok != error_my_id)
        {
            TSLogging::Error("DumpTalkStatusChanges", m_meTalkingScHandler, error_my_id);
            return;
        }
        iTalk->onTalkStatusChanged(m_meTalkingScHandler, status, m_meTalkingIsWhisper, my_id, true);
    }
}

bool Talkers::onTalkStatusChangeEvent(uint64 connection_id, int status, int isReceivedWhisper, anyID client_id)
{
    const auto [error_my_id, my_id] = funcs::get_client_id(connection_id);
    if (ts_errc::ok != error_my_id)
    {
        TSLogging::Error("onTalkStatusChangeEvent", connection_id, error_my_id);
        return false;
    }

    if (my_id == client_id)
    {
        m_meTalkingIsWhisper = isReceivedWhisper;
        if (status == STATUS_TALKING)
            m_meTalkingScHandler = connection_id;
        else
            m_meTalkingScHandler = 0;

        return true;
    }

    if (status == STATUS_TALKING)
    {
        if (isReceivedWhisper)
        {
            if (!WhisperMap.contains(connection_id, client_id))   // pure safety measurement
                WhisperMap.insert(connection_id, client_id);
        }
        else
        {
            if (!TalkerMap.contains(connection_id, client_id))
                TalkerMap.insert(connection_id, client_id);
        }
    }
    else if (status == STATUS_NOT_TALKING)
    {
        if (isReceivedWhisper)
            WhisperMap.remove(connection_id, client_id);
        else
            TalkerMap.remove(connection_id,client_id);
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
