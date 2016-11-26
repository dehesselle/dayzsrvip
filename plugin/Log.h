/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef LOG_H
#define LOG_H

#ifndef DAYZSRVIP_LIBRARY
#define ELPP_QT_LOGGING
#define ELPP_THREAD_SAFE
#define ELPP_NO_DEFAULT_LOG_FILE
#include "easylogging++.h"
#else

#include <QString>
#include "ts3_functions.h"

void setTs3FunctionsForLog(const struct TS3Functions funcs);
void logDebug(QString message);
void logInfo(QString message);
void logError(QString message);

#endif   // DAYZSRVIP_LIBRARY

#endif // LOG_H
