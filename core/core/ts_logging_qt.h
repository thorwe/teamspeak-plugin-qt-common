#pragma once

#ifndef PATH_BUFSIZE
#define PATH_BUFSIZE 512
#endif

#include <QtCore/QString>

#include "teamspeak/public_definitions.h"

namespace TSLogging
{
    bool GetErrorSound(QString &in);
    bool GetInfoIcon(QString &in);
    void PlayErrorSound(uint64 serverConnectionHandlerID = 0);

    void Error(QString message, uint64 serverConnectionHandlerID, unsigned int error, bool isSoundSilent = false);
    inline void Error(QString message, unsigned int error)                                      {Error(message, 0, error, false);}
    inline void Error(QString message, bool isSoundSilent)                                      {Error(message, 0, NULL, isSoundSilent);}
    inline void Error(QString message)                                                          {Error(message, 0, NULL, false);}

    void Print(QString message, uint64 serverConnectionHandlerID = 0, LogLevel logLevel = LogLevel_INFO);
    inline void Print(QString message, LogLevel logLevel)                                       {Print(message, 0, logLevel);}

    void Log(QString message, uint64 serverConnectionHandlerID = 0, LogLevel logLevel = LogLevel_INFO);
    inline void Log(QString message, LogLevel logLevel)                                         {Log(message, 0, logLevel);}
}
