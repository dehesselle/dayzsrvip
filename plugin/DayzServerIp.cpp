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
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
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

   logDebug("m_settings.openFile()");
   m_settings.openFile(m_path + "/dayzsrvip.ini");

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   ui->rbOff->setChecked(true);
   ui->rbOff->setToolTip("stop sending your data to everyone in your channel");
   ui->rbOn->setEnabled(false);
   ui->rbOn->setToolTip("start sending your data to everyone in your channel");

   ui->pbRequestSitrep->setEnabled(false);
   ui->pbRequestSitrep->setToolTip("request everyone to send an update");
   ui->pbRemoteInfoClear->setToolTip("clear the list");
   ui->pbOpenProfile->setToolTip("select your DayZ profile file");
   ui->pbOpenLog->setToolTip("open TeamSpeak log");

   ui->gvLogo->setToolTip("DayZ is the game!");

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

   logDebug("setupRemoteInfo()");
   setupRemoteInfo();   // setup columns in treeview

   // import history from file
   {
      QFile history(m_path + "/dayzsrvip.hst");

      if (history.open(QFile::ReadOnly))
      {
         logDebug("importing history from: " + history.fileName());

         QTextStream in(&history);

         while (! in.atEnd())
         {
            QString line = in.readLine();
            updateRemoteInfo(line, false);
         }
      }
      else
      {
         logDebug("no history to import");
      }
   }

   setStatusMessage(QString("Welcome! This is version ")
                    + DAYZSERVERIP_VERSION
                    + QString("."));

   {
      QString dayzProfile = m_settings.value(Player::INI_DAYZ_PROFILE).toString();

      if (QFile::exists(dayzProfile))
      {
         if (m_player.importFromFile(dayzProfile))
         {
            updateLocalInfo(m_player.toLocalInfo());
            m_fsWatcher->addPath(dayzProfile);
            ui->rbOn->setEnabled(true);
         }
         // no 'else': we don't treat this as error here while still
         // constructing; m_player will have logged an error though
      }
      else
      {
         logInfo("profile does not exist (not an error at this point): " + dayzProfile);
      }
   }
}

DayzServerIp::~DayzServerIp()
{
   delete ui;
}

void DayzServerIp::on_pbOpenProfile_clicked()
{
   QString dayzProfile =
         QFileDialog::getOpenFileName(this,
                                      "Open DayZ Profile",
                                      QStandardPaths::locate(QStandardPaths::DocumentsLocation,
                                                             QString(),
                                                             QStandardPaths::LocateDirectory) + "/DayZ",
                                      "DayZ Profile (*.DayZProfile)");

   if (QFile::exists(dayzProfile))
   {
      if (m_player.importFromFile(dayzProfile))
      {
         m_settings.setValue(Player::INI_DAYZ_PROFILE, dayzProfile);

         if (m_player.m_isChanged)
            updateLocalInfo(m_player.toLocalInfo());

         if (m_fsWatcher->files().count())
            m_fsWatcher->removePaths(m_fsWatcher->files());

         m_fsWatcher->addPath(dayzProfile);
         ui->rbOn->setEnabled(true);
      }
      else
      {
         QMessageBox::critical(this,
                               "invalid profile",
                               "This DayZProfile cannot be used!\n\n"
                               "The most likely cause for this error is \n"
                               "not having set a character name in DayZ.");
         ui->rbOn->setEnabled(false);
         m_fsWatcher->removePaths(m_fsWatcher->files());
      }
   }
   else   // file dialog has been cancelled
   {
      ui->rbOn->setEnabled(false);
      m_fsWatcher->removePaths(m_fsWatcher->files());
   }
}

void DayzServerIp::updateRemoteInfo(QString info,
                                    bool saveInfo)
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

         sortRemoteInfo();
         if (saveInfo)
            saveRemoteInfo(info);
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
      logError("unable to import DayZProfile after 5 attempts");
   }
   else
   {
//      logDebug("import count = " + QString::number(count));
   }

   if (! m_fsWatcher->files().count())
      m_fsWatcher->addPath(path);

   if (m_player.m_isChanged)
   {
      updateLocalInfo(m_player.toLocalInfo());

      if (ui->rbOn->isChecked())
      {
         requestSendTs3Message(m_player.toMessage());
         setStatusMessage("Sent update to teammates.");
      }
   }
}

void DayzServerIp::on_rbOn_clicked()
{
   ui->pbOpenProfile->setEnabled(false);
   ui->pbRequestSitrep->setEnabled(true);
   setStatusMessage("meddel on!");
}

void DayzServerIp::on_rbOff_clicked()
{
   ui->pbOpenProfile->setEnabled(true);
   ui->pbRequestSitrep->setEnabled(false);
   setStatusMessage("meddel off!");
}

void DayzServerIp::on_pbRemoteInfoClear_clicked()
{
   QString history = m_path + "/dayzsrvip.hst";

   if (! QFile::remove(history))
      logError("failed to remove history file: " + history);

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
         if (ui->rbOn->isChecked())
         {
            setStatusMessage("Responding to request for sitrep.");
            emit sendTs3Message(m_player.toMessage());
         }
         else
         {
            setStatusMessage("Ignoring request for sitrep (we're still OFF).");
         }
         break;
      case MessageType::INVALID:
         break;
   }
}

void DayzServerIp::setStatusMessage(const QString &message)
{
   ui->lMessage->setText(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + message);
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

void DayzServerIp::on_pbRequestSitrep_clicked()
{
   setStatusMessage("Requesting sitrep from teammates.");
   requestSendTs3Message(MSG_STR_REQUEST_SITREP);
}

void DayzServerIp::saveRemoteInfo(const QString &text)
{
   QFile history(m_path + "/dayzsrvip.hst");

   if (history.open(QFile::Append))
   {
      QTextStream out(&history);
      out << text << "\n";
   }
   else
   {
      logError("failed to write history: " + history.fileName());
   }
}

void DayzServerIp::on_pbOpenLog_clicked()
{
   QDir logDir(m_path + "/logs");
   QString currentLog = logDir.entryList(QStringList() << "*.log",
                                         QDir::Files,
                                         QDir::Time).at(0);

   if (! currentLog.isEmpty())
   {
      // We need to recreate the file with CRLF so we can
      // open it with notepad.exe
      currentLog = logDir.path() + "/" + currentLog;
      QFile fileIn(currentLog);
      QFile fileOut(currentLog + ".txt");

      if (fileIn.open(QFile::ReadOnly) &&
          fileOut.open(QFile::WriteOnly))
      {
         QTextStream streamOut(&fileOut);

         while (! fileIn.atEnd())
            streamOut << fileIn.readLine() << "\r\n";
      }

      fileIn.close();
      fileOut.close();

      QProcess::startDetached("notepad.exe "
                              + fileOut.fileName());
   }
   else
   {
      QMessageBox::information(this,
                               "log not found",
                               "It looks like there is no log.",
                               QMessageBox::Ok);
   }
}
