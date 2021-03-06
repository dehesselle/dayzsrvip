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

const IniFile::KeyValue Player::INI_DAYZPROFILE = { "Player/dayzProfile", "" };

const char* Player::XML_PLAYER = "player";
const char* Player::XML_TS3NAME = "ts3name";
const char* Player::XML_DAYZNAME = "dayzname";
const char* Player::XML_SERVERNAME = "servername";
const char* Player::XML_SERVERIP = "serverip";
const char* Player::XML_TIMESTAMP = "timestamp";
const char* Player::XML_PLAYER_VERSION = "version";
const char* Player::XML_PLAYER_VERSION_VALUE = "1";

const char* Player::DAYZPROFILE_LASTMPSERVER = "lastMPServer";
const char* Player::DAYZPROFILE_LASTMPSERVERNAME = "lastMPServerName";
const char* Player::DAYZPROFILE_PLAYERNAME = "playerName";

// These intial values have to be the same ones
// used in res/player_template.html.
const char* Player::INIT_SERVERNAME = "___SERVER_NAME___";
const char* Player::INIT_SERVERIP = "___SERVER_IP___";
const char* Player::INIT_DAYZNAME = "___DAYZ_NAME___";
const char* Player::INIT_TS3NAME = "___TS3_NAME___";

Player::Player() :
   m_isChanged(false)
{
   initialize();
}

bool Player::fromDayzProfile(QString filename)
{
   bool result = false;

   m_filename = filename;

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

            // not looking for DAYZPROFILE_LASTMPSERVERNAME because first
            // part of this string is identical to DAYPROFILE_LASTMPSERVER
            if (line.contains(DAYZPROFILE_PLAYERNAME) ||
                line.contains(DAYZPROFILE_LASTMPSERVER))
               profileData << line;
         }

         if (profileData.count() == 3)   // we're supposed to have found 3 items
         {
            QRegExp regex("=|\"");   // split the line by '=' or '"'

            for (int i = 0; i < profileData.count(); i++)
            {
               QStringList parts = profileData.at(i).split(regex);
               if      (parts.at(0) == DAYZPROFILE_PLAYERNAME)
               {
                  setDayzName(parts.at(2));
                  logDebug("fromDayzProfile() "
                           + QString(DAYZPROFILE_PLAYERNAME)
                           + " " + getDayzName());
               }
               else if (parts.at(0) == DAYZPROFILE_LASTMPSERVER)
               {
                  setServerIp(parts.at(2));
                  logDebug("fromDayzProfile() "
                           + QString(DAYZPROFILE_LASTMPSERVER)
                           + " " + getServerIp());
               }
               else if (parts.at(0) == DAYZPROFILE_LASTMPSERVERNAME)
               {
                  setServerName(parts.at(2));
                  logDebug("fromDayzProfile() "
                        + QString(DAYZPROFILE_LASTMPSERVERNAME)
                        + " " + getServerName());
               }
            }

            updateTimestamp();
            updateChangedFlag();

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

void Player::updateChangedFlag()
{
   if (getDayzName()   == getDayzNameOld() &&
       getServerName() == getServerNameOld() &&
       getServerIp()   == getServerIpOld())
   {
      m_isChanged = false;
      logDebug("no relevant changes to profile");
   }
   else
   {
      m_isChanged = true;
      setDayzNameOld(getDayzName());
      setServerNameOld(getServerName());
      setServerIpOld(getServerIp());
      logDebug("relevant changes to profile detected");
   }
}

void Player::toXml(QXmlStreamWriter& xml) const
{
   xml.writeStartElement(XML_PLAYER);
   xml.writeAttribute(XML_PLAYER_VERSION, XML_PLAYER_VERSION_VALUE);

   foreach (const QString& key, m_data.keys())
      xml.writeTextElement(key, m_data.value(key));

   xml.writeEndElement();   // STR_PLAYER
}

void Player::fromXml(QXmlStreamReader& xml)
{
   if (xml.isStartElement() &&
       xml.name() == XML_PLAYER &&
       xml.attributes().hasAttribute(XML_PLAYER_VERSION) &&
       xml.attributes().value(XML_PLAYER_VERSION) == XML_PLAYER_VERSION_VALUE)
   {
      while (! (xml.isEndElement() && xml.name() == XML_PLAYER) )
      {
         xml.readNext();

         if (xml.isStartElement())
         {
            QString key(xml.name().toString());
            xml.readNext();
            if(xml.isCharacters())
               m_data[key] = xml.text().toString();
         }
      }
   }
   else
   {
      logError("Player::fromXml() unable to parse");
   }
}

void Player::updateTimestamp()
{
   setTimestamp(QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
}

void Player::setDayzName(const QString &dayzName)
{
   m_data[XML_DAYZNAME] = dayzName;
}

void Player::setTs3Name(const QString &ts3Name)
{
   m_data[XML_TS3NAME] = ts3Name;
}

void Player::setServerName(const QString &serverName)
{
   m_data[XML_SERVERNAME] = serverName;
}

void Player::setServerIp(const QString &serverIp)
{
   m_data[XML_SERVERIP] = serverIp;
}

void Player::setTimestamp(const QString &timestamp)
{
   m_data[XML_TIMESTAMP] = timestamp;
}

QString Player::getDayzName() const
{
   return m_data[XML_DAYZNAME];
}

QString Player::getTs3Name() const
{
   return m_data[XML_TS3NAME];
}

QString Player::getServerName() const
{
   return m_data[XML_SERVERNAME];
}

QString Player::getServerIp() const
{
   return m_data[XML_SERVERIP];
}

QString Player::getTimestamp() const
{
   return m_data[XML_TIMESTAMP];
}

const bool& Player::isChanged()
{
   return m_isChanged;
}

void Player::setDayzNameOld(const QString &dayzName)
{
   m_dataOld[XML_DAYZNAME] = dayzName;
}

void Player::setServerNameOld(const QString &serverName)
{
   m_dataOld[XML_SERVERNAME] = serverName;
}

void Player::setServerIpOld(const QString &serverIp)
{
   m_dataOld[XML_SERVERIP] = serverIp;
}

QString Player::getDayzNameOld() const
{
   return m_dataOld[XML_DAYZNAME];
}

QString Player::getServerNameOld() const
{
   return m_dataOld[XML_SERVERNAME];
}

QString Player::getServerIpOld() const
{
   return m_dataOld[XML_SERVERIP];
}

bool Player::hasDayzProfile() const
{
   bool result = false;

   if (getDayzName() == INIT_DAYZNAME &&
       getServerIp() == INIT_SERVERIP &&
       getServerName() == INIT_SERVERNAME)
      result = false;
   else
      result = true;

   return result;
}

void Player::initialize()
{
   setDayzName(INIT_DAYZNAME);
   setServerIp(INIT_SERVERIP);
   setServerName(INIT_SERVERNAME);
   setTs3Name(INIT_TS3NAME);

   setDayzNameOld(INIT_DAYZNAME);
   setServerIpOld(INIT_SERVERIP);
   setServerNameOld(INIT_SERVERNAME);

   updateTimestamp();
}
