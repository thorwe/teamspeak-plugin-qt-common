#pragma once

#include "core/definitions.h"
#include "core/module.h"
#include "core/ts_error.h"

#include "teamspeak/public_definitions.h"

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

    void Print(const QString &message,
               com::teamspeak::connection_id_t connection_id = 0,
               LogLevel logLevel = LogLevel_INFO);
    inline void Print(const QString& message, LogLevel logLevel) { Print(message, 0 ,logLevel); }
    void Log(const QString &message,
             com::teamspeak::connection_id_t connection_id = 0,
             LogLevel logLevel = LogLevel_INFO);
    inline void Log(const QString& message, LogLevel logLevel)   { Log(message, 0, logLevel); }
    void Error(const QString &message,
               com::teamspeak::connection_id_t connection_id = 0,
               std::error_code error = ts_errc::ok);
    inline void Error(const QString &message, std::error_code error) { Error(message, 0, error); }

    bool m_isPrintEnabled{ false };
};
