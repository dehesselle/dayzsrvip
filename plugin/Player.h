/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "IniFile.h"

class Player
{
public:
   Player();

   static const IniFile::KeyValue INI_DAYZPROFILE;

   // These are the strings used when construction the XML and "the relevant
   // ones" are also used as key to access m_data/m_dataOld.
   static const char* XML_PLAYER;   ///< XML tag
   static const char* XML_PLAYER_VERSION;
   static const char* XML_PLAYER_VERSION_VALUE;
   static const char* XML_TS3NAME;
   static const char* XML_DAYZNAME;
   static const char* XML_SERVERNAME;
   static const char* XML_SERVERIP;
   static const char* XML_TIMESTAMP;

   // These contain the exact strings we're looking for in .DayzProfile.
   static const char* DAYZPROFILE_PLAYERNAME;         ///< playerName
   static const char* DAYZPROFILE_LASTMPSERVER;       ///< lastMPServer (IP)
   static const char* DAYZPROFILE_LASTMPSERVERNAME;   ///< lastMPServerName

   static const char* INIT_SERVERNAME;   ///< initial value setServerName
   static const char* INIT_SERVERIP;     ///< initial value setServerIp
   static const char* INIT_DAYZNAME;     ///< initial value setDayzName
   static const char* INIT_TS3NAME;      ///< initial value setTs3Name

   bool fromDayzProfile(QString filename);
   void toXml(QXmlStreamWriter& xml) const;
   void fromXml(QXmlStreamReader& xml);

   void setDayzName(const QString& dayzName);
   void setTs3Name(const QString& ts3Name);
   void setServerName(const QString& serverName);
   void setServerIp(const QString& serverIp);
   void setTimestamp(const QString& timestamp);
   void updateTimestamp();

   QString getDayzName() const;
   QString getTs3Name() const;
   QString getServerName() const;
   QString getServerIp() const;
   QString getTimestamp() const;

   const bool& isChanged();
   bool hasDayzProfile() const;
   void initialize();

private:
   void setDayzNameOld(const QString& dayzName);
   void setServerNameOld(const QString& serverName);
   void setServerIpOld(const QString& serverIp);

   QString getDayzNameOld() const;
   QString getServerNameOld() const;
   QString getServerIpOld() const;

   void updateChangedFlag();

   typedef QMap<QString,QString> Data;
   Data m_data;      ///< current player data
   Data m_dataOld;   ///< previous player data (i.e. one update ago)

   QString m_filename;   ///< .DayzProfile
   bool m_isChanged;     ///< m_data is different from m_dataOld
};

#endif // PLAYER_H
