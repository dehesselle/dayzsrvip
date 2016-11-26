#include "Log.h"

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
