#include "core/talkers.h"

#include "core/ts_functions.h"
#include "core/ts_logging_qt.h"

#include "plugin.h"

#include <algorithm>

using namespace com::teamspeak::pluginsdk;

Talkers::Talkers(QObject* parent)
	: QObject(parent)
{}

std::error_code Talkers::RefreshTalkers(com::teamspeak::connection_id_t connection_id)
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

std::error_code Talkers::RefreshAllTalkers()
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

    {
        auto lock = std::shared_lock<decltype(m_mutex)>{};
        {
            for (const auto &talker : m_talkers)
            {
                iTalk->onTalkStatusChanged(talker.connection_id, status, talker.is_whispering,
                                           talker.client_id, false);
            }
        }

        if (m_me_talking_connection_id != 0)
        {
            const auto [error_my_id, my_id] = funcs::get_client_id(m_me_talking_connection_id);
            if (ts_errc::ok != error_my_id)
            {
                TSLogging::Error("DumpTalkStatusChanges", m_me_talking_connection_id, error_my_id);
                return;
            }
            iTalk->onTalkStatusChanged(m_me_talking_connection_id, status, m_me_talking_is_whisper, my_id,
                                       true);
        }
    }
}

bool Talkers::onTalkStatusChangeEvent(com::teamspeak::connection_id_t connection_id,
                                      int status,
                                      int is_received_whisper,
                                      com::teamspeak::client_id_t client_id)
{
    const auto [error_my_id, my_id] = funcs::get_client_id(connection_id);
    if (ts_errc::ok != error_my_id)
    {
        TSLogging::Error("onTalkStatusChangeEvent", connection_id, error_my_id);
        return false;
    }

    {
        auto lock = std::unique_lock<decltype(m_mutex)>{};
        if (my_id == client_id)
        {
            m_me_talking_is_whisper = is_received_whisper;
            if (status == STATUS_TALKING)
                m_me_talking_connection_id = connection_id;
            else
                m_me_talking_connection_id = 0;

            return true;
        }

        if (status == STATUS_TALKING)
        {
            if (auto it = std::find_if(std::begin(m_talkers), std::end(m_talkers),
                                       [connection_id, client_id](const auto &talker) {
                                           return talker.connection_id == connection_id &&
                                                  talker.client_id == client_id;
                                       });
                it != std::end(m_talkers))
            {
                if (it->is_whispering != !!is_received_whisper)
                    it->is_whispering = !!is_received_whisper;
            }
            else
            {
                m_talkers.emplace_back(Talkers_Info{connection_id, client_id, !!is_received_whisper});
            }
        }
        else if (status == STATUS_NOT_TALKING)
        {
            if (auto it = std::find_if(std::begin(m_talkers), std::end(m_talkers),
                                       [connection_id, client_id](const auto &talker) {
                                           return talker.connection_id == connection_id &&
                                                  talker.client_id == client_id;
                                       });
                it != std::end(m_talkers))
            {
                m_talkers.erase(it);
            }
        }
    }
    return false;
}

void Talkers::onConnectStatusChangeEvent(com::teamspeak::connection_id_t connection_id,
                                         int new_status,
                                         unsigned int /*error_number*/)
{
    auto fake_end = decltype(m_talkers){};
    if (new_status == STATUS_DISCONNECTED)
    {
        auto lock = std::unique_lock<decltype(m_mutex)>{};
        std::remove_copy_if(
        std::begin(m_talkers), std::end(m_talkers), std::begin(fake_end),
        [connection_id](const auto &talker) { return talker.connection_id != connection_id; });
    }
    for (const auto &talker : fake_end)
        ts3plugin_onTalkStatusChangeEvent(connection_id, TalkStatus::STATUS_NOT_TALKING,
                                          talker.is_whispering ? 1 : 0, talker.client_id);
}

com::teamspeak::connection_id_t Talkers::isMeTalking() const
{
    auto lock = std::shared_lock<decltype(m_mutex)>{};
    return m_me_talking_connection_id;
}
