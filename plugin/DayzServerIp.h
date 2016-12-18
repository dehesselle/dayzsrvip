/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef DAYZSERVERIP_H
#define DAYZSERVERIP_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStringList>
#include <QString>
#include <QFileSystemWatcher>
#include <QGraphicsScene>
#include <QXmlStreamReader>
#include "IniFile.h"
#include "Player.h"

namespace Ui {
class DayzServerIp;
}

class DayzServerIp : public QDialog
{
   Q_OBJECT

public:
   explicit DayzServerIp(QWidget *parent = 0,
                         const QString& configPath = "");
   ~DayzServerIp();

   void setTs3Name(const QString& name);

   enum PLC   // Player List Columns
   {
      PLC_TS3NAME = 0,   ///< nickname in TeamSpeak
      PLC_DAYZNAME,      ///< DayZ: playerName
      PLC_SERVERNAME,    ///< DayZ: lastMpServerName
      PLC_SERVERIP,      ///< DayZ: lastMpServer
      PLC_TIMESTAMP,     ///< last update
      PLC_COUNT          ///< no. of elements in this enum
   };

   static const char* XML_NAME;             ///< tag
   static const char* XML_VERSION;          ///< attribute name
   static const char* XML_VERSION_VALUE;    ///< attribute value
   static const char* XML_COMMAND;          ///< attribute name
   static const char* XML_COMMAND_SITREP;   ///< attribute value
   static const char* XML_COMMAND_UPDATE;   ///< attribute value

   static const IniFile::KeyValue INI_VERSIONNO;     ///< (git) version
   static const IniFile::KeyValue INI_RUNCOUNT;      ///< usage counter
   static const IniFile::KeyValue INI_CHATENABLED;   ///< send text messages
   static const IniFile::KeyValue INI_GEOMETRY;      ///< window geometry

   static const IniFile::KeyValue INI_COL_TS3NAME;      ///< column width
   static const IniFile::KeyValue INI_COL_DAYZNAME;     ///< column width
   static const IniFile::KeyValue INI_COL_SERVERNAME;   ///< column width
   static const IniFile::KeyValue INI_COL_SERVERIP;     ///< column width
   static const IniFile::KeyValue INI_COL_TIMESTAMP;    ///< column width

signals:
   void sendTs3Message(QString text);
   void sendTs3Command(QString command);

private slots:
   void on_cbChat_toggled(bool checked);

   void on_pbProfileOpen_clicked();
   void on_pbRemoteInfoClear_clicked();
   void on_pbSitrepRequest_clicked();

   void on_rbOff_clicked();
   void on_rbOn_clicked();

   void onFsWatcherFileChanged(const QString& path);

public slots:
   void onTs3CommandReceived(const QString& command);

private:
   void savePlayerListEntry(const Player& player);
   void setupPlayerList();
   void sortPlayerList();
   void updatePlayerList(const Player& player, bool saveToFile);

   void setStatusMessage(const QString& message);
   void requestSendTs3Command(const QString& command);
   void checkVersionNo();
   void updatePlayer();

   void updateRunCount(int count = 0);
   int getRunCount();

   void processProfile(const QString& filename,
                       bool forceUpdate = false);

   QString createUpdateCommand();
   QString createUpdateMessage();
   QString createSitrepCommand();

   Ui::DayzServerIp* ui;

   QGraphicsScene* m_scene;   ///< show DayZ logo in GUI

   QStandardItemModel m_playerListModel;   ///< data of all users playing DayZ
   QString m_playerListFile;   ///< history of all users playing DayZ

   IniFile m_settings;   ///< our persistent settings

   QFileSystemWatcher* m_fsWatcher;   ///< watch .DayZProfile for changes

   Player m_player;        ///< data of local user playing DayZ
   QString m_playerHtml;   ///< content of res/player_template.html for tbPlayer

   QString m_configPath;   ///< TeamSpeak's configuration directory
};

#endif // DAYZSERVERIP_H
