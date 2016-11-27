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

   enum class MessageType
   {
      INVALID = 0,
      UPDATE_SERVER,
      RENAME_CHAR,
      REQUEST_SITREP
   };

   MessageType toMessageType(const QStringList& message);

   enum USMF   // Update Server Message Fields
   {
      USMF_INGAME_NAME = 0,
      USMF_TS3_NAME,
      USMF_SERVER_NAME,
      USMF_SERVER_IP,
      USMF_TIMESTAMP,
      USMF_COUNT
   };

   enum RCMF   // Rename Char Message Fields
   {
      RCMF_NAME_OLD = 0,
      RCMF_NAME_NEW,
      RCMF_COUNT
   };

   enum LIF   // Local Info Fields
   {
      LIF_INGAME_NAME = 0,
      LIF_SERVER_NAME,
      LIF_SERVER_IP,
      LIF_COUNT
   };

   static const char* MSG_STR_UPDATE_SERVER;
   static const char* MSG_STR_RENAME_CHAR;
   static const char* MSG_STR_REQUEST_SITREP;
   static const char* MSG_STR_SEPARATOR;

   static const IniFile::KeyValue INI_VERSION_NO;

   friend class DebugDialog;

signals:
   void sendTs3Message(QString text);

private slots:
   void on_pbDebugOpen_clicked();
   void on_pbLogOpen_clicked();
   void on_rbOff_clicked();
   void on_rbOn_clicked();
   void on_pbProfileOpen_clicked();
   void on_pbRemoteInfoClear_clicked();
   void on_pbSitrepRequest_clicked();

   void onFsWatcherFileChanged(const QString& path);

public slots:
   void onTs3MessageReceived(const QString& message);

private:
   void setupRemoteInfo();
   void sortRemoteInfo();
   void saveRemoteInfo(const QString& text);

   void setStatusMessage(const QString& message);
   void requestSendTs3Message(const QString& message);
   void checkVersionNo();
   void updateRemoteInfo(QString info, bool saveInfo);
   void updateLocalInfo(QStringList info);

   void processProfile(const QString& filename,
                       bool forceUpdate = false);

   Ui::DayzServerIp *ui;

   QGraphicsScene* m_scene;

   QStandardItemModel m_remoteInfo;
   QString m_remoteInfoFile;   // historical data

   IniFile m_settings;

   QFileSystemWatcher* m_fsWatcher;

   Player m_player;

   QString m_configPath;
};

#endif // DAYZSERVERIP_H
