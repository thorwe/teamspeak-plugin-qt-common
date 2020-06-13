#include "core/module_qt.h"

#include "core/ts_logging_qt.h"

void Module_Qt::setEnabled(bool val)
{
    Module::set_enabled(val);
}

void Module_Qt::setBlocked(bool val)
{
    Module::set_blocked(val);
}

void Module_Qt::on_enabled_changed(bool val)
{
    onEnabledChanged(val);
    emit enabledSet(val);
}

void Module_Qt::on_blocked_changed(bool val)
{
    onBlockedChanged(val);
    emit blockedSet(val);
}

void Module_Qt::on_running_changed(bool val)
{
    onRunningChanged(val);
}

void Module_Qt::Print(const QString &message, uint64 connection_id, LogLevel logLevel)
{
#ifndef CONSOLE_OUTPUT
    Q_UNUSED(message);
    Q_UNUSED(connection_id);
    Q_UNUSED(logLevel);
#else
    if (!m_isPrintEnabled)
        return;

    TSLogging::Print((this->objectName() + ": " + message), connection_id, logLevel);
#endif
}

void Module_Qt::Log(const QString &message, uint64 connection_id, LogLevel logLevel)
{
    TSLogging::Log((this->objectName() + ": " + message), connection_id, logLevel);
    Print(message, connection_id, logLevel);
}

void Module_Qt::Error(const QString &message, uint64 connection_id, std::error_code error)
{
    TSLogging::Error((this->objectName() + ": " + message), connection_id, error);
}
