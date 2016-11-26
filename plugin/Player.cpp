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

Player::Player() :
   m_isChanged(false)
{

}

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

         if (profileData.count() == 3)
         {
            QRegExp regex("=|\"");

            for (int i = 0; i < profileData.count(); i++)
            {
               QStringList parts = profileData.at(i).split(regex);
               if      (parts.at(0) == "playerName")
               {
                  logDebug("found: playerName");
                  m_name = parts.at(2);
               }
               else if (parts.at(0) == "lastMPServer")
               {
                  logDebug("found: lastMPServer");
                  m_serverIp = parts.at(2);
               }
               else if (parts.at(0) == "lastMPServerName")
               {
                  logDebug("found: lastMPServerName");
                  m_serverName = parts.at(2);
               }
            }

            updateTimestamp();
            updateChanged();

            result = true;
         }
         else
         {
            logError("profileData.count() == "
                     + QString::number(profileData.count())
                     + ", but must be 3");
         }

         dayzProfile.close();
      }
   }

   return result;
}

void Player::updateChanged()
{
   if (m_name == m_nameOld &&
       m_serverName == m_serverNameOld &&
       m_serverIp == m_serverIpOld)
   {
      m_isChanged = false;
      logDebug("no relevant changes to profile");
   }
   else
   {
      m_isChanged = true;
      m_nameOld = m_name;
      m_serverNameOld = m_serverName;
      m_serverIpOld = m_serverIp;
      logDebug("relevant changes to profile detected");
   }
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
