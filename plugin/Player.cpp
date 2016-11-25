/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "Player.h"
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QDateTime>
#include "Log.h"
#include "DayzServerIp.h"

const IniFile::KeyValue Player::INI_DAYZ_PROFILE = { "Player/dayzProfile", "" };

bool Player::importFromFile(QString filename)
{
   bool result = false;

   m_filename = filename;

#ifndef DAYZSRVIP_LIBRARY
   LOG(TRACE) << "filename: " << m_filename;
#endif

   if (QFile::exists(m_filename))
   {
      QFile dayzProfile(m_filename);

      if (dayzProfile.open(QFile::ReadOnly))
      {
         QTextStream in(&dayzProfile);

         QStringList profileData;
         while (! in.atEnd())
         {
            QString line = in.readLine();

            if (line.contains("playerName") ||
                line.contains("lastMPServer"))
               profileData << line;
         }
#ifndef DAYZSRVIP_LIBRARY
         LOG(TRACE) << "profileData.count() = " << profileData.count();
#endif
         if (profileData.count() == 3)
         {
            QRegExp regex("=|\"");

            for (int i = 0; i < profileData.count(); i++)
            {
               QStringList parts = profileData.at(i).split(regex);
               if      (parts.at(0) == "playerName")
                  m_name = parts.at(2);
               else if (parts.at(0) == "lastMPServer")
                  m_serverIp = parts.at(2);
               else if (parts.at(0) == "lastMPServerName")
                  m_serverName = parts.at(2);
            }

            updateTimestamp();

#ifndef DAYZSRVIP_LIBRARY
            LOG(TRACE) << "m_name(" << m_name << ")";
            LOG(TRACE) << "m_serverIp(" << m_serverIp << ")";
            LOG(TRACE) << "m_serverName(" << m_serverName << ")";
            LOG(TRACE) << "m_timestamp(" << m_timestamp << ")";
#endif
            result = true;
         }

         dayzProfile.close();
      }
   }

   return result;
}

QString Player::toMessage()
{
   QString result;
   updateTimestamp();

   result = QString(DayzServerIp::MSG_STR_UPDATE_SERVER) + "###"
         + m_name + "###"
         + "TS3NAME_PLACEHOLDER###"
         + m_serverName + "###"
         + m_serverIp + "###"
         + m_timestamp;

   return result;
}

QStringList Player::toLocalInfo()
{
   QStringList result;

   result << m_name
          << m_serverName
          << m_serverIp;

   return result;
}

void Player::updateTimestamp()
{
   m_timestamp = QDateTime::currentDateTime().toString("yy.MM.dd hh:mm:ss");
}
