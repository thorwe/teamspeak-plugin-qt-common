#include <QtCore/QObject>
#include <QtCore/QPointer>

#include "core/ts_infodata_qt.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "ts3_functions.h"
#include "plugin.h"

#include "core/ts_logging_qt.h"

const int kInfoDataBufSize = 256;

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
            ts3Functions.requestInfoUpdate(server_connection_id, m_info_type, m_info_id);

        break;
    case PLUGIN_CHANNEL:
        if (m_info_id == id)
            ts3Functions.requestInfoUpdate(server_connection_id, m_info_type, m_info_id);

        break;
    case PLUGIN_SERVER:
        ts3Functions.requestInfoUpdate(server_connection_id, m_info_type, m_info_id);
        break;
    default:
        break;
    }
}

void TSInfoData::RequestSelfUpdate()
{
    if (m_home_id != 0)
    {
        unsigned int error;
        int status;
        if ((error = ts3Functions.getConnectionStatus(m_home_id, &status)) != ERROR_ok)
        {
            TSLogging::Error("(TSInfoData::RequestSelfUpdate)", NULL, error, false);
            return;
        }

		// Get My Id on this handler
        anyID my_id;
        if((error = ts3Functions.getClientID(m_home_id, &my_id)) != ERROR_ok)
        {
            TSLogging::Error("(TSInfoData::RequestSelfUpdate)", m_home_id, error);
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

void TSInfoData::onInfoData(uint64 server_connection_id, uint64 id, enum PluginItemType type, char** data)
{
    // When disconnecting a server tab, an info update will be sent with this server_connection_id
    // That's rather not helpfull
    unsigned int error;
    int con_status;
    if ((error = ts3Functions.getConnectionStatus(server_connection_id, &con_status)) != ERROR_ok)
    {
        TSLogging::Error("(TSInfoData::onInfoData)", server_connection_id, error);
        return;
    }
    if (con_status == STATUS_DISCONNECTED)
        return;

    m_home_id = server_connection_id;
    m_info_type = type;
    m_info_id = id;

    uint64 mine = 0;
    if ((type == PLUGIN_CLIENT) || (type == PLUGIN_CHANNEL))
    {
        unsigned int error;
        // Get My Id on this handler
        anyID my_id;
        if((error = ts3Functions.getClientID(server_connection_id, &my_id)) != ERROR_ok)
        {
            if (error != ERROR_not_connected)
                TSLogging::Error("(TSInfoData::onInfoData)", server_connection_id, error);

            return;
        }

        if (type == PLUGIN_CLIENT)
            mine = static_cast<uint64>(my_id);
        else
        {
            // Get My channel on this handler
            uint64 channelID;
            if ((error = ts3Functions.getChannelOfClient(server_connection_id, my_id, &channelID)) != ERROR_ok)
            {
                TSLogging::Error("(TSInfoData::onInfoData)", server_connection_id, error);
                return;
            }
            mine = channelID;
        }
    }

    QString outstr;
    QTextStream data_stream(&outstr);
    for (auto it = m_callbacks.constBegin(); it != m_callbacks.constEnd(); ++it)
    {
        auto cbk = it.value();
        if (cbk)
        {
            auto i_info = qobject_cast<InfoDataInterface *>(cbk);
            if (i_info->onInfoDataChanged(server_connection_id, id, type, mine, data_stream))
                data_stream << ".\n";
        }
    }

    if (outstr.isEmpty())
        data = NULL;
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
