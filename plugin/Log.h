/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef LOG_H
#define LOG_H

#ifdef DAYZSRVIP_LIBRARY

#include <QString>
#include "ts3_functions.h"

void setTs3FunctionsForLog(const struct TS3Functions funcs);

#else

#include <QDebug>

#endif   // DAYZSRVIP_LIBRARY

void logDebug(QString message);
void logInfo(QString message);
void logWarning(QString message);
void logError(QString message);

void disableDebugMessages();
void enableDebugMessages();

#endif // LOG_H
