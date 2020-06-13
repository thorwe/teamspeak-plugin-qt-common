#pragma once

#include "core/ts_error.h"

#include "teamspeak/public_definitions.h"

#include <QtCore/QString>

namespace TSLogging
{
    bool GetErrorSound(QString &in);
    bool GetInfoIcon(QString &in);
    void PlayErrorSound(uint64 serverConnectionHandlerID = 0);

    void Error(QString message, uint64 connection_id, std::error_code error, bool isSoundSilent = false);
    inline void Error(QString message, std::error_code error)
    {
        Error(message, 0, error, false);
    }
    inline void Error(QString message, bool isSoundSilent)
    {
        Error(message, 0, ts_errc::ok, isSoundSilent);
    }
    inline void Error(QString message)
    {
        Error(message, 0, ts_errc::ok, false);
    }

    void Print(QString message, uint64 serverConnectionHandlerID = 0, LogLevel logLevel = LogLevel_INFO);
    inline void Print(QString message, LogLevel logLevel)                                       {Print(message, 0, logLevel);}

    void Log(QString message, uint64 serverConnectionHandlerID = 0, LogLevel logLevel = LogLevel_INFO);
    inline void Log(QString message, LogLevel logLevel)                                         {Log(message, 0, logLevel);}
}
