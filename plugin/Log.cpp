#include "Log.h"

static bool isDebugEnabled = false;

void disableDebugMessages()
{
   ::isDebugEnabled = false;
}

void enableDebugMessages()
{
   ::isDebugEnabled = true;
}

#ifdef DAYZSRVIP_LIBRARY

static struct TS3Functions ts3Functions;

void setTs3FunctionsForLog(const struct TS3Functions funcs)
{
   ts3Functions = funcs;
}

void logDebug(QString message)
{
   ::isDebugEnabled && ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_DEBUG, "dayzsrvip", 0);
}

void logInfo(QString message)
{
   ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_INFO, "dayzsrvip", 0);
}

void logWarning(QString message)
{
   ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_WARNING, "dayzsrvip", 0);
}

void logError(QString message)
{
   ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_ERROR, "dayzsrvip", 0);
}

#else

void logDebug(QString message)
{
   if (::isDebugEnabled)
      qDebug() << message;
}

void logInfo(QString message)
{
   qInfo() << message;
}

void logWarning(QString message)
{
   qWarning() << message;
}

void logError(QString message)
{
   qCritical() << message;
}

#endif
