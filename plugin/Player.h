/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QStringList>
#include "IniFile.h"

class Player
{
public:
   static const IniFile::KeyValue INI_DAYZ_PROFILE;

   bool importFromFile(QString filename);
   QString toString();
   QStringList toStringList();

   QString m_name;
   QString m_serverName;
   QString m_serverIp;
   QString m_timestamp;
};

#endif // PLAYER_H
