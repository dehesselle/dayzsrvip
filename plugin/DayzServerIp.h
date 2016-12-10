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

   enum RIC   // Remote Info Columns
   {
      RIC_TS3_NAME = 0,
      RIC_INGAME_NAME,
      RIC_SERVER_NAME,
      RIC_SERVER_IP,
      RIC_TIMESTAMP,
      RIC_COUNT
   };

   static const char* PLAYER_SERVERNAME_INIT;
   static const char* PLAYER_SERVERIP_INIT;
   static const char* PLAYER_INGAMENAME_INIT;
   static const char* PLAYER_TS3NAME_INIT;

   static const char* XML_NAME;
   static const char* XML_VERSION;
   static const char* XML_VERSION_VALUE;
   static const char* XML_COMMAND;
   static const char* XML_COMMAND_SITREP;
   static const char* XML_COMMAND_UPDATE;

   static const IniFile::KeyValue INI_VERSION_NO;
   static const IniFile::KeyValue INI_RUN_COUNT;
   static const IniFile::KeyValue INI_CHAT_ENABLED;

signals:
   void sendTs3Message(QString text);
   void sendTs3Command(QString command);

private slots:
   void on_pbLogOpen_clicked();
   void on_rbOff_clicked();
   void on_rbOn_clicked();
   void on_pbProfileOpen_clicked();
   void on_pbRemoteInfoClear_clicked();
   void on_pbSitrepRequest_clicked();

   void onFsWatcherFileChanged(const QString& path);

   void on_cbChat_toggled(bool checked);

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

   void processProfile(const QString& filename,
                       bool forceUpdate = false);

   QString createUpdateCommand();
   QString createUpdateMessage();
   QString createSitrepCommand();

   Ui::DayzServerIp* ui;

   QGraphicsScene* m_scene;

   QStandardItemModel m_playerListModel;
   QString m_playerListFile;

   IniFile m_settings;

   QFileSystemWatcher* m_fsWatcher;

   Player m_player;

   QString m_configPath;
};

#endif // DAYZSERVERIP_H
