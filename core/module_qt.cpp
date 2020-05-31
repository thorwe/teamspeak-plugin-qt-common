#include "core/module_qt.h"
#include "ts3_functions.h"
#include "plugin.h"

#include <QtCore/QTime>
#include <QtCore/QTextStream>

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

void Module_Qt::Print(QString message, uint64 serverConnectionHandlerID, LogLevel logLevel)
{
#ifndef CONSOLE_OUTPUT
    Q_UNUSED(message);
    Q_UNUSED(serverConnectionHandlerID);
    Q_UNUSED(logLevel);
#else
    if (!m_isPrintEnabled)
        return;

    TSLogging::Print((this->objectName() + ": " + message), serverConnectionHandlerID, logLevel);
#endif
}

void Module_Qt::Log(QString message, uint64 serverConnectionHandlerID, LogLevel logLevel)
{
    TSLogging::Log((this->objectName() + ": " + message), serverConnectionHandlerID, logLevel);
    Print(message, serverConnectionHandlerID, logLevel);
}

void Module_Qt::Error(QString message, uint64 serverConnectionHandlerID, unsigned int error)
{
    TSLogging::Error((this->objectName() + ": " + message), serverConnectionHandlerID, error);
}
