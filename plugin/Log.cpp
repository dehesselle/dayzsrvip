#include "Log.h"

#ifdef DAYZSRVIP_LIBRARY

static struct TS3Functions ts3Functions;

void setTs3FunctionsForLog(const struct TS3Functions funcs)
{
   ts3Functions = funcs;
}

void logDebug(QString message)
{
   ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_DEBUG, "dayzsrvip", 0);
}
void logInfo(QString message)
{
   ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_INFO, "dayzsrvip", 0);
}
void logError(QString message)
{
   ts3Functions.logMessage(message.toStdString().c_str(), LogLevel_ERROR, "dayzsrvip", 0);
}

#else

void logDebug(QString message)
{
   qDebug() << message;
}
void logInfo(QString message)
{
   qInfo() << message;
}
void logError(QString message)
{
   qCritical() << message;
}

#endif
