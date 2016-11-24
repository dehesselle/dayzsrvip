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
#include "IniFile.h"
#include "Player.h"
#include "GraphicsScene.h"   // also includes QGraphicsScene

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

   void updateRemoteInfo(QString info, bool saveHistory);
   void updateLocalInfo(QStringList info);

   enum class MessageType
   {
      INVALID = 0,
      UPDATE_SERVER,
      RENAME_CHAR,
      REQUEST_SITREP,
   };

   MessageType toMessageType(const QStringList& message);

   static const char* MSG_STR_UPDATE_SERVER;
   static const char* MSG_STR_RENAME_CHAR;
   static const char* MSG_STR_REQUEST_SITREP;
   static const char* MSG_STR_SEPARATOR;

signals:
   void sendTs3Message(QString text);

private slots:
   void on_pbOpenProfile_clicked();
   void on_pbRemoteInfoClear_clicked();
   void on_pbSitrep_clicked();

   void on_rbOn_clicked();
   void on_rbOff_clicked();

   void onFsWatcherFileChanged(const QString& path);

public slots:
   void onTs3MessageReceived(const QString& message);

private:
   void setupRemoteInfo();
   void setStatusMessage(const QString& message);
   void requestSendTs3Message(const QString& message);

   Ui::DayzServerIp *ui;

#ifdef QT_DEBUG
   GraphicsScene*  m_scene;
#else
   QGraphicsScene* m_scene;
#endif

   QStandardItemModel m_remoteInfo;

   IniFile m_settings;

   QFileSystemWatcher* m_fsWatcher;

   Player m_player;

   QString m_path;
};

#endif // DAYZSERVERIP_H
