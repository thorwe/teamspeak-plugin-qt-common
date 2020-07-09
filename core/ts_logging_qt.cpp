#include "core/ts_logging_qt.h"

#include "core/ts_functions.h"
#include "core/ts_helpers_qt.h"
#include "core/ts_settings_qt.h"

#include "plugin.h"

using namespace com::teamspeak::pluginsdk;

bool TSLogging::GetErrorSound(QString &in)
{
    QString pack;
    if (TSSettings::instance()->GetSoundPack(pack))
    {
        // Find the path to the soundpack
        QString path_qstr = TSHelpers::GetPath(teamspeak::plugin::Path::Resources);
        path_qstr.append("sound/" + pack);

        QSettings cfg(path_qstr + "/settings.ini", QSettings::IniFormat);
        auto snd_qstr = cfg.value("soundfiles/SERVER_ERROR").toString();
        if (!snd_qstr.isEmpty())
        {
            // towatch: QSettings insists on eliminating the double quotas '\"' on read
            // no, I won't spend one more minute creating a regexp that fits for a fragging error sound that should be available via the api.
            snd_qstr.remove("play(");
            snd_qstr.remove(")");

            path_qstr.append("/" + snd_qstr);
            in = path_qstr;
        }
        return true; // Here so that the user setting of "No Sound" (and speech synthesis? don't throw errors)
    }
    return false;
}

bool TSLogging::GetInfoIcon(QString &in)
{
    QString pack;
    if (TSSettings::instance()->GetIconPack(pack))
    {
        // Find the path to the skin
        QString path_qstr = TSHelpers::GetPath(teamspeak::plugin::Path::Resources);
        in = (path_qstr + "gfx/" + pack + "/16x16_message_info.png");
        return true;
    }
    return false;
}

void TSLogging::PlayErrorSound(uint64 connection_id)
{
    if (0 == connection_id)
        connection_id = funcs::get_current_server_connection_handler_id();

    if (0 == connection_id)
        return;

    QString errorSound;
    if (!(GetErrorSound(errorSound)))
        Log(("TSLogging::PlayErrorSound:" + TSSettings::instance()->GetLastError().text()), LogLevel_WARNING);
    else
    {
        if(!errorSound.isEmpty())
        {
            if (const auto error_play =
                funcs::sound::play_wave_file(connection_id, errorSound.toLocal8Bit().constData());
                ts_errc::ok != error_play)
            {
                Log(
                QString("Error playing error sound: %1").arg(QString::fromStdString(error_play.message())),
                LogLevel_WARNING);
            }
        }
    }
}

void TSLogging::Error(QString message, uint64 connection_id, std::error_code error, bool isSoundSilent)
{

    if (ts_errc::ok != error)
        QTextStream(&message) << ": " << error.message().data();

    Log(message, connection_id, LogLevel_ERROR);
    if (!isSoundSilent)
        PlayErrorSound(connection_id);

    // Format and print the error message
    if (0 == connection_id)
        connection_id = funcs::get_current_server_connection_handler_id();

    if (0 == connection_id)
        return;

    // Use a transparent underscore because a double space will be collapsed
    QString styledQstr;

    /*QString infoIcon;
    if (!(GetInfoIcon(infoIcon)))
    {
        //Get Error
        QString msg = "TSLogging::Error:GetInfoIcon:" + TSSettings::instance()->GetLastError().text();
        ts_funcs.logMessage(msg.toLocal8Bit().constData(), LogLevel_ERROR, ts3plugin_name(), 0);
    }
    else
        QTextStream(&styledQstr) << "[img]" << infoIcon << "[/img]";*/

    QTime time = QTime::currentTime ();
    auto time_qstr = time.toString(Qt::TextDate);

    QTextStream(&styledQstr) << "[color=red]" << time_qstr << "[/color][color=transparent]_[/color]" << ": "  << ts3plugin_name() << ": " << message;
    funcs::print_message(connection_id, styledQstr.toLocal8Bit().constData(), PLUGIN_MESSAGE_TARGET_SERVER);
}

void TSLogging::Print(const QString &message, uint64 connection_id, LogLevel logLevel)
{
#ifndef CONSOLE_OUTPUT
    Q_UNUSED(message);
    Q_UNUSED(connection_id);
    Q_UNUSED(logLevel);
#else
    switch (logLevel)
    {
    case LogLevel_CRITICAL:
        message.prepend("Critical: ");
        break;
    case LogLevel_DEBUG:
        message.prepend("Debug: ");
        break;
    case LogLevel_DEVEL:
        message.prepend("Devel: ");
        break;
    case LogLevel_ERROR:
        message.prepend("Error: ");
        break;
    case LogLevel_INFO:
        message.prepend("Info: ");
        break;
    case LogLevel_WARNING:
        message.prepend("Warning: ");
        break;
    }

    if (connection_id != NULL)
        message.prepend(QString("%1 : ").arg(connection_id));

    auto time = QTime::currentTime();
    auto time_qstr = time.toString(Qt::TextDate);
    QString styledQstr;
    QTextStream(&styledQstr) << time_qstr << ": " << ts3plugin_name() << ": " << message << "\n";
    printf(styledQstr.toLocal8Bit().constData());
#endif
}

void TSLogging::Log(const QString &message, uint64 connection_id, LogLevel logLevel)
{
    funcs::log_message(message.toLocal8Bit().constData(), logLevel, ts3plugin_name(), connection_id);
}
