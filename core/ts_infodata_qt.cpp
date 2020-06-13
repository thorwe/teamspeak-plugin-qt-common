#include "core/ts_infodata_qt.h"

#include "core/ts_functions.h"
#include "core/ts_logging_qt.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>

const int kInfoDataBufSize = 256;

using namespace com::teamspeak::pluginsdk;

TSInfoData::TSInfoData(QObject *parent)
	: QObject(parent)
{}

PluginItemType TSInfoData::getInfoType() const
{
    return m_info_type;
}

uint64 TSInfoData::getInfoId() const
{
    return m_info_id;
}

void TSInfoData::RequestUpdate(uint64 server_connection_id, uint64 id, enum PluginItemType type)
{
    if ((m_info_id == 0) || (type != m_info_type) || (server_connection_id != m_home_id))
        return;

    switch(type)
    {
    case PLUGIN_CLIENT:
        if (m_info_id == id)
            funcs::request_info_update(server_connection_id, m_info_type, m_info_id);

        break;
    case PLUGIN_CHANNEL:
        if (m_info_id == id)
            funcs::request_info_update(server_connection_id, m_info_type, m_info_id);

        break;
    case PLUGIN_SERVER:
        funcs::request_info_update(server_connection_id, m_info_type, m_info_id);
        break;
    default:
        break;
    }
}

void TSInfoData::RequestSelfUpdate()
{
    const auto home_id = m_home_id;
    if (home_id != 0)
    {
        const auto [error_connection_status, connection_status] = funcs::get_connection_status(home_id);
        if (ts_errc::ok != error_connection_status)
        {
            TSLogging::Error("(TSInfoData::RequestSelfUpdate)", NULL, error_connection_status, false);
            return;
        }

		// Get My Id on this handler
        const auto[error_my_id, my_id] = funcs::get_client_id(home_id);
        if (ts_errc::ok != error_my_id)
        {
            TSLogging::Error("(TSInfoData::RequestSelfUpdate)", home_id, error_my_id);
            return;
        }

        if (my_id == static_cast<anyID>(m_info_id))
            RequestUpdate(m_home_id, my_id, PLUGIN_CLIENT);
    }
}

bool TSInfoData::Register(QObject *p, bool is_register, int priority)
{
    auto iInfoData = qobject_cast<InfoDataInterface *>(p);
    if (!iInfoData)
        return false;

    if (is_register)
    {
        if (m_callbacks.values().contains(QPointer<QObject>(p)))
            return true;

        m_callbacks.insert(priority, QPointer<QObject>(p));
    }
    else
    {
        for (auto it = m_callbacks.begin(); it != m_callbacks.end();)
        {
            if (it.value() == QPointer<QObject>(p))
                it = m_callbacks.erase(it);
            else
                ++it;
        }
    }
    return true;
}

void TSInfoData::onInfoData(uint64 connection_id, uint64 id, enum PluginItemType type, char** data)
{
    // When disconnecting a server tab, an info update will be sent with this server_connection_id
    // That's rather not helpfull
    const auto [error_connection_status, connection_status] = funcs::get_connection_status(connection_id);
    if (ts_errc::ok != error_connection_status)
    {
        TSLogging::Error("(TSInfoData::onInfoData)", connection_id, error_connection_status);
        return;
    }
    if (STATUS_DISCONNECTED == connection_status)
        return;

    m_home_id = connection_id;
    m_info_type = type;
    m_info_id = id;

    uint64 mine = 0;
    if ((type == PLUGIN_CLIENT) || (type == PLUGIN_CHANNEL))
    {
        const auto[error_my_id, my_id] = funcs::get_client_id(connection_id);
        if (ts_errc::ok != error_my_id)
        {
            if (ts_errc::not_connected != error_my_id)
                TSLogging::Error("(TSInfoData::onInfoData)", connection_id, error_my_id);

            return;
        }

        if (type == PLUGIN_CLIENT)
            mine = static_cast<uint64>(my_id);
        else
        {
            const auto [error_my_channel_id, my_channel_id] = funcs::get_channel_of_client(connection_id, my_id);
            if (ts_errc::ok != error_my_channel_id)
            {
                TSLogging::Error("(TSInfoData::onInfoData)", connection_id, error_my_channel_id);
                return;
            }
            mine = my_channel_id;
        }
    }

    QString outstr;
    QTextStream data_stream(&outstr);
    for (auto it = m_callbacks.constBegin(); it != m_callbacks.constEnd(); ++it)
    {
        auto&& cbk = it.value();
        if (cbk)
        {
            auto i_info = qobject_cast<InfoDataInterface *>(cbk);
            if (i_info->onInfoDataChanged(connection_id, id, type, mine, data_stream))
                data_stream << ".\n";
        }
    }

    if (outstr.isEmpty())
        data = nullptr;
    else
    {
        *data = (char*)malloc(kInfoDataBufSize * sizeof(char));
        if (outstr.size() > kInfoDataBufSize - 1)
        {
            TSLogging::Error("Infodata too long for teamspeak. Shortening.",true);
            outstr = outstr.trimmed();
            if (outstr.size() > kInfoDataBufSize - 1)
                outstr.truncate(kInfoDataBufSize - 1);
        }
        qstrncpy(*data, outstr.toLatin1().constData(), kInfoDataBufSize - 1);
    }
}
