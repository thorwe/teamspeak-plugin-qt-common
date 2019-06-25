#include "core/module.h"
#include "ts3_functions.h"
#include "plugin.h"

#include <QtCore/QTime>
#include <QtCore/QTextStream>

#include "core/ts_logging_qt.h"

//Module::Module(QObject *parent) :
//    QObject(parent)
//{
//}

bool Module::isEnabled() const
{
    return m_enabled.load();
}

bool Module::isBlocked() const
{
    return m_blocked.load();
}

bool Module::isRunning() const
{
    return (m_enabled.load() && !m_blocked.load());
}


void Module::setEnabled(bool value)
{
    if (value != m_enabled.load())
    {
        const auto kRunningOld = isRunning();
        m_enabled.store(value);
        onEnabledStateChanged(value);
        if (kRunningOld != isRunning())
            onRunningStateChanged(isRunning());

        emit enabledSet(value);
    }
}

void Module::setBlocked(bool value)
{
    if (value != m_blocked.load())
    {
        const auto kRunningOld = isRunning();
        m_blocked.store(value);
        onBlockedStateChanged(value);
        if (kRunningOld != isRunning())
            onRunningStateChanged(isRunning());

        emit blockedSet(value);
    }
}

void Module::Print(QString message, uint64 serverConnectionHandlerID, LogLevel logLevel)
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

void Module::Log(QString message, uint64 serverConnectionHandlerID, LogLevel logLevel)
{
    TSLogging::Log((this->objectName() + ": " + message),serverConnectionHandlerID,logLevel);
    Print(message, serverConnectionHandlerID, logLevel);
}

void Module::Error(QString message, uint64 serverConnectionHandlerID, unsigned int error)
{
    TSLogging::Error((this->objectName() + ": " + message), serverConnectionHandlerID, error);
}
