#pragma once

#include "core/module.h"

#include <QtCore/QObject>

class Module_Qt : public QObject, public Module
{
    Q_OBJECT
    Q_PROPERTY(bool enabled
               READ enabled
               WRITE setEnabled
               NOTIFY enabledSet)
    Q_PROPERTY(bool blocked
               READ blocked
               WRITE setBlocked
               NOTIFY blockedSet)

signals:
    void enabledSet(bool);
    void blockedSet(bool);

public:
    void setEnabled(bool value);
    void setBlocked(bool value);

protected:
    void on_enabled_changed(bool) final override;
    void on_blocked_changed(bool) final override;
    void on_running_changed(bool) final override;

    virtual void onEnabledChanged(bool) {}
    virtual void onBlockedChanged(bool) {}
    virtual void onRunningChanged(bool) {}

    void Print(QString message, uint64 serverConnectionHandlerID = 0, LogLevel logLevel = LogLevel_INFO);
    inline void Print(QString message, LogLevel logLevel)   {Print(message, 0 ,logLevel);}
    void Log(QString message, uint64 serverConnectionHandlerID = 0, LogLevel logLevel = LogLevel_INFO);
    inline void Log(QString message, LogLevel logLevel)     {Log(message, 0, logLevel);}
    void Error(QString message, uint64 serverConnectionHandlerID = 0, unsigned int error = ERROR_ok);
    inline void Error(QString message, unsigned int error)  {Error(message, 0, error);}

    bool m_isPrintEnabled{ false };
};
