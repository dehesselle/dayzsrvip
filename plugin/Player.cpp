/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "Player.h"
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>
#include <QDateTime>
#include "Log.h"

const IniFile::KeyValue Player::INI_DAYZ_PROFILE = { "Player/dayzProfile", "" };

bool Player::importFromFile(QString filename)
{
   LOG(TRACE) << "filename: " << filename;

   bool result = false;

   if (QFile::exists(filename))
   {
      QFile dayzProfile(filename);

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

         LOG(TRACE) << "profileData.count() = " << profileData.count();

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

            m_timestamp = QFileInfo(filename).lastModified().toString("yy.MM.dd hh:mm:ss");

            LOG(TRACE) << "m_name(" << m_name << ")";
            LOG(TRACE) << "m_serverIp(" << m_serverIp << ")";
            LOG(TRACE) << "m_serverName(" << m_serverName << ")";
            LOG(TRACE) << "m_timestamp(" << m_timestamp << ")";

            result = true;
         }

         dayzProfile.close();
      }
   }

   return result;
}

QString Player::toString()
{
   QString result;

   result = "[dayzsrvip]###"
         + m_name + "###"
         + m_serverName + "###"
         + m_serverIp + "###"
         + m_timestamp;

   return result;
}

QStringList Player::toStringList()
{
   QStringList result;

   result << m_name
          << m_serverName
          << m_serverIp;

   return result;
}
