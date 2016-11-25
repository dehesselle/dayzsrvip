/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "DayzServerIp.h"
#include "ui_DayzServerIp.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QList>
#include <QThread>
#include <QMessageBox>
#include <QDateTime>
#include "Version.h"
#include "Log.h"

static const char* LOCALINFO_SERVER_INIT = "___SERVER_NAME___";
static const char* LOCALINFO_SERVER_IP_INIT = "___SERVER_IP___";
static const char* LOCALINFO_CHAR_NAME_INIT = "___CHAR_NAME___";

const char* DayzServerIp::MSG_STR_UPDATE_SERVER = "[dayzsrvip|server]";
const char* DayzServerIp::MSG_STR_RENAME_CHAR = "[dayzsrvip|char]";
const char* DayzServerIp::MSG_STR_REQUEST_SITREP = "[dayzsrvip|sitrep]";
const char* DayzServerIp::MSG_STR_SEPARATOR = "###";

DayzServerIp::DayzServerIp(QWidget *parent,
                           const QString& configPath) :
   QDialog(parent),
   ui(new Ui::DayzServerIp),
   m_scene(0),
   m_fsWatcher(new QFileSystemWatcher(this)),
   m_path(configPath.left(configPath.length() - 1))
{
   Q_INIT_RESOURCE(dayzsrvip);

   ui->setupUi(this);

   connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &DayzServerIp::onFsWatcherFileChanged);

   m_settings.openFile(m_path + "/dayzsrvip.ini");

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   ui->rbOff->setChecked(true);
   ui->rbOn->setEnabled(false);

   ui->pbSitrep->setEnabled(false);
   ui->pbSitrep->setToolTip("Request update everyone in your channel.");

   // show DayZ Logo
   {
#ifdef QT_DEBUG
      // this enables the 'DebugDialog': left-click on the DayZ-Logo ('gvLogo')
      m_scene = new GraphicsScene(this);
#else
      m_scene = new QGraphicsScene(this);
#endif
      m_scene->addPixmap(QPixmap(":/dayzlogo"));
      ui->gvLogo->setScene(m_scene);
   }

   setupRemoteInfo();   // setup columns in treeview

   // import history from file
   {
      QFile history(m_path + "/dayzsrvip.hst");

      if (history.open(QFile::ReadOnly))
      {
         QTextStream in(&history);

         while (! in.atEnd())
         {
            QString line = in.readLine();
            updateRemoteInfo(line, false);
         }
      }
   }

   setStatusMessage(QString("Welcome! This is version ")
                    + DAYZSERVERIP_VERSION
                    + QString("."));

   if (m_player.importFromFile(m_settings.value(Player::INI_DAYZ_PROFILE).toString()))
   {
      updateLocalInfo(m_player.toLocalInfo());
      m_fsWatcher->addPath(m_settings.value(Player::INI_DAYZ_PROFILE).toString());
      ui->rbOn->setEnabled(true);
      ui->pbSitrep->setEnabled(true);
   }
}

DayzServerIp::~DayzServerIp()
{
   delete ui;
}

void DayzServerIp::on_pbOpenProfile_clicked()
{
   QString filename =
         QFileDialog::getOpenFileName(this,
                                      "Open DayZ Profile",
                                      QStandardPaths::locate(QStandardPaths::DocumentsLocation,
                                                             QString(),
                                                             QStandardPaths::LocateDirectory) + "/DayZ",
                                      "DayZ Profile (*.DayZProfile)");


   if (m_player.importFromFile(filename))
   {
      m_settings.setValue(Player::INI_DAYZ_PROFILE, filename);
      updateLocalInfo(m_player.toLocalInfo());

      if (m_fsWatcher->files().count())
      {
         m_fsWatcher->removePaths(m_fsWatcher->files());
         m_fsWatcher->addPath(filename);
      }

      ui->rbOn->setEnabled(true);
   }
}

void DayzServerIp::updateRemoteInfo(QString info,
                                    bool saveHistory)
{
#ifndef DAYZSRVIP_LIBRARY
   LOG(TRACE) << "info(" << info.length() << "): " << info;
#endif
   QRegExp regex(MSG_STR_SEPARATOR);
   QStringList infoFields = info.split(regex);

   switch (toMessageType(infoFields))
   {
      case MessageType::UPDATE_SERVER:
      {
         infoFields.pop_front();   // lose the message ID
         QString name = infoFields.at(USMF_INGAME_NAME);
         QList<QStandardItem*> itemList = m_remoteInfo.findItems(name);

         switch (itemList.count())
         {
            case 0:   // insert new item
            {
#ifndef DAYZSRVIP_LIBRARY
               LOG(TRACE) << "inserting item";
#endif
               int row = m_remoteInfo.rowCount();

               for (int i = 0; i < infoFields.count(); i++)
               {
                  QStandardItem* item = new QStandardItem(infoFields.at(i));
                  m_remoteInfo.setItem(row, i, item);
               }
               break;
            }
            case 1:   // update existing item
            {
#ifndef DAYZSRVIP_LIBRARY
               LOG(TRACE) << "updating item";
#endif
               QStandardItem* firstItem = itemList.at(USMF_INGAME_NAME);
               int row = firstItem->row();

               QList<QStandardItem*> items;
               for (int col = 0; col < m_remoteInfo.columnCount(); col++)
                  if (col)
                     items << m_remoteInfo.item(row, col)->clone();
                  else
                     items << new QStandardItem(QString(""));

               firstItem->insertRow(0, items);

               for (int col = 1; col < m_remoteInfo.columnCount(); col++)
                  m_remoteInfo.item(row, col)->setText(infoFields.at(col));

               break;
            }
            default:
               break;
         }

         if (saveHistory)
         {
            QFile history(m_path + "/dayzsrvip.hst");

            if (history.open(QFile::Append))
            {
               QTextStream out(&history);
               out << info << "\n";
            }
         }

         sortRemoteInfo();
         break;
      }
      case MessageType::RENAME_CHAR:
      {
         infoFields.pop_front();   // lose the message ID

         QString nameOld = infoFields.at(RCMF_NAME_OLD);
         QString nameNew = infoFields.at(RCMF_NAME_NEW);

         QList<QStandardItem*> itemList = m_remoteInfo.findItems(nameOld);

         for (QList<QStandardItem*>::Iterator it = itemList.begin();
              it != itemList.end();
              it++)
         {
            (*it)->setText(nameNew);
         }

         //sortRemoteInfo();
         break;
      }
      case MessageType::REQUEST_SITREP:
      case MessageType::INVALID:
      {
         break;
      }
   }
}

void DayzServerIp::updateLocalInfo(QStringList info)
{
   static QString oldServerName = LOCALINFO_SERVER_INIT;
   static QString oldServerIp = LOCALINFO_SERVER_IP_INIT;
   static QString oldCharName = LOCALINFO_CHAR_NAME_INIT;

   QString charName = info.at(LIF_INGAME_NAME);
   QString serverName = info.at(LIF_SERVER_NAME);
   QString serverIp = info.at(LIF_SERVER_IP);

   QString html = ui->tbLocalInfo->toHtml();
   html.replace(oldCharName, charName);
   html.replace(oldServerName, serverName);
   html.replace(oldServerIp, serverIp);
   ui->tbLocalInfo->setHtml(html);

   if ((oldCharName != charName) &&
      (oldCharName != LOCALINFO_CHAR_NAME_INIT))
   {
      requestSendTs3Message(QString(MSG_STR_RENAME_CHAR)
                            + MSG_STR_SEPARATOR + oldCharName
                            + MSG_STR_SEPARATOR + charName);
   }

   oldCharName = charName;
   oldServerName = serverName;
   oldServerIp = serverIp;
}

void DayzServerIp::onFsWatcherFileChanged(const QString& path)
{
#ifndef DAYZSRVIP_LIBRARY
   LOG(TRACE) << "file: " << path;
#endif

   int count = 0;

   while (count < 6)
   {
      QThread::sleep(1);

      if (m_player.importFromFile(path))
         break;

      count++;
   }

   if (count > 5)
   {
#ifndef DAYZSRVIP_LIBRARY
      LOG(ERROR) << "unable to import after " << count << " attempts";
#endif
      QMessageBox::critical(this,
                            "read error", "Unable to read DayZ Profile.",
                            QMessageBox::Ok);
   }

   if (! m_fsWatcher->files().count())
      m_fsWatcher->addPath(path);

   updateLocalInfo(m_player.toLocalInfo());

   requestSendTs3Message(m_player.toMessage());
   setStatusMessage("Sent update to teammates.");
}

void DayzServerIp::on_rbOn_clicked()
{
   ui->pbOpenProfile->setEnabled(false);
   ui->pbSitrep->setEnabled(true);
   setStatusMessage("ON");
}

void DayzServerIp::on_rbOff_clicked()
{
   ui->pbOpenProfile->setEnabled(true);
   ui->pbSitrep->setEnabled(false);
   setStatusMessage("OFF");
}

void DayzServerIp::on_pbRemoteInfoClear_clicked()
{
   m_remoteInfo.clear();
   setupRemoteInfo();
}

DayzServerIp::MessageType DayzServerIp::toMessageType(const QStringList& message)
{
   MessageType result = MessageType::INVALID;
   int count = message.count() - 1;   // remove message ID

   if (message.at(0) == MSG_STR_UPDATE_SERVER)
   {
      if (count == USMF_COUNT)
         result = MessageType::UPDATE_SERVER;
   }
   else if (message.at(0) == MSG_STR_RENAME_CHAR)
   {
      if (count == RCMF_COUNT)
         result = MessageType::RENAME_CHAR;
   }
   else if (message.at(0) == MSG_STR_REQUEST_SITREP)
   {
      result = MessageType::REQUEST_SITREP;
   }

   return result;
}

void DayzServerIp::setupRemoteInfo()
{
   m_remoteInfo.setHorizontalHeaderItem(USMF_INGAME_NAME, new QStandardItem("in-game"));
   m_remoteInfo.setHorizontalHeaderItem(USMF_TS3_NAME, new QStandardItem("TS3"));
   m_remoteInfo.setHorizontalHeaderItem(USMF_SERVER_NAME, new QStandardItem("Server"));
   m_remoteInfo.setHorizontalHeaderItem(USMF_SERVER_IP, new QStandardItem("IP"));
   m_remoteInfo.setHorizontalHeaderItem(USMF_TIMESTAMP, new QStandardItem("Timestamp"));
   ui->tvRemoteInfo->setModel(&m_remoteInfo);
   ui->tvRemoteInfo->setColumnWidth(USMF_INGAME_NAME, 105);
   ui->tvRemoteInfo->setColumnWidth(USMF_TS3_NAME, 90);
   ui->tvRemoteInfo->setColumnWidth(USMF_SERVER_NAME, 180);
   ui->tvRemoteInfo->setColumnWidth(USMF_SERVER_IP, 90);
   ui->tvRemoteInfo->setColumnWidth(USMF_TIMESTAMP, 80);
}

void DayzServerIp::onTs3MessageReceived(const QString &message)
{
   QRegExp regex(MSG_STR_SEPARATOR);
   QStringList messageParts = message.split(regex);

   switch (toMessageType(messageParts))
   {
      case MessageType::UPDATE_SERVER:
         updateRemoteInfo(message, true);
         setStatusMessage("Update from teammate received.");
         break;
      case MessageType::RENAME_CHAR:
         updateRemoteInfo(message, false);
         setStatusMessage("Teammate changed name.");
         break;
      case MessageType::REQUEST_SITREP:
         setStatusMessage("Request for sitrep received.");
         emit sendTs3Message(m_player.toMessage());
         break;
      case MessageType::INVALID:
         break;
   }
}

void DayzServerIp::setStatusMessage(const QString &message)
{
   ui->lMessage->setText(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + message);
}

void DayzServerIp::on_pbSitrep_clicked()
{
   setStatusMessage("Requesting sitrep from teammates.");
   requestSendTs3Message(MSG_STR_REQUEST_SITREP);
}

void DayzServerIp::requestSendTs3Message(const QString &message)
{
   if (ui->rbOn->isChecked())
      emit sendTs3Message(message);
   else
      setStatusMessage("Not transmitting - still 'off'.");
}

void DayzServerIp::sortRemoteInfo()
{
   m_remoteInfo.sort(ui->tvRemoteInfo->header()->sortIndicatorSection(),
                     ui->tvRemoteInfo->header()->sortIndicatorOrder());
}
