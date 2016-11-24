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
#include "Version.h"
#include "Log.h"

static const char* LOCALINFO_SERVER_INIT = "___SERVER_NAME___";
static const char* LOCALINFO_SERVER_IP_INIT = "___SERVER_IP___";
static const char* LOCALINFO_CHAR_NAME_INIT = "___CHAR_NAME___";

const char* DayzServerIp::MSG_STR_UPDATE_SERVER = "[dayzsrvip|server]";
const char* DayzServerIp::MSG_STR_RENAME_CHAR = "[dayzsrvip|char]";

DayzServerIp::DayzServerIp(QWidget *parent,
                           const QString& configPath) :
   QDialog(parent),
   ui(new Ui::DayzServerIp),
   m_scene(0),
   m_fsWatcher(new QFileSystemWatcher(this)),
   m_path(configPath.left(configPath.length() - 1))
{
   Q_INIT_RESOURCE(dayzsrvip);

   connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &DayzServerIp::onFsWatcherFileChanged);

   ui->setupUi(this);

   m_settings.openFile(m_path + "/dayzsrvip.ini");

   setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

   ui->rbOff->setChecked(true);
   ui->rbOn->setEnabled(false);

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

   ui->lMessage->setText(QString("version ") + DAYZSERVERIP_VERSION);

   if (m_player.importFromFile(m_settings.value(Player::INI_DAYZ_PROFILE).toString()))
   {
      updateLocalInfo(m_player.toLocalInfo());
      m_fsWatcher->addPath(m_settings.value(Player::INI_DAYZ_PROFILE).toString());
      ui->rbOn->setEnabled(true);
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
   QRegExp regex("###");
   QStringList infoList = info.split(regex);

   switch (toMessage(infoList))
   {
      case MessageType::UPDATE_SERVER:
      {
         infoList.pop_front();   // lose the message ID
         QString name = infoList.at(0);   // first item is char name
         QList<QStandardItem*> itemList = m_remoteInfo.findItems(name);

         switch (itemList.count())
         {
            case 0:   // insert new item
            {
#ifndef DAYZSRVIP_LIBRARY
               LOG(TRACE) << "inserting item";
#endif
               int row = m_remoteInfo.rowCount();

               for (int i = 0; i < infoList.count(); i++)
               {
                  QStandardItem* item = new QStandardItem(infoList.at(i));
                  m_remoteInfo.setItem(row, i, item);
               }
               break;
            }
            case 1:   // update existing item
            {
#ifndef DAYZSRVIP_LIBRARY
               LOG(TRACE) << "updating item";
#endif
               QStandardItem* firstItem = itemList.at(0);
               int row = firstItem->row();

               QList<QStandardItem*> items;
               for (int col = 0; col < m_remoteInfo.columnCount(); col++)
                  if (col)
                     items << m_remoteInfo.item(row, col)->clone();
                  else
                     items << new QStandardItem(QString(""));

               firstItem->insertRow(0, items);

               for (int col = 1; col < m_remoteInfo.columnCount(); col++)
                  m_remoteInfo.item(row, col)->setText(infoList.at(col));

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
         break;
      }
      case MessageType::RENAME_CHAR:
      {
         infoList.pop_front();   // lose the message ID

         QString nameOld = infoList.at(0);
         QString nameNew = infoList.at(1);

         QList<QStandardItem*> itemList = m_remoteInfo.findItems(nameOld);

         for (QList<QStandardItem*>::Iterator it = itemList.begin();
              it != itemList.end();
              it++)
         {
            (*it)->setText(nameNew);
         }
         break;
      }
      case MessageType::INVALID:
      {
         break;
      }
   }

   if (infoList.at(0) == MSG_STR_UPDATE_SERVER)
   {
      m_remoteInfo.sort(ui->tvRemoteInfo->header()->sortIndicatorSection(),
                        ui->tvRemoteInfo->header()->sortIndicatorOrder());
   }
}

void DayzServerIp::updateLocalInfo(QStringList info)
{
   static QString oldServerName = LOCALINFO_SERVER_INIT;
   static QString oldServerIp = LOCALINFO_SERVER_IP_INIT;
   static QString oldCharName = LOCALINFO_CHAR_NAME_INIT;

   QString charName = info.at(0);
   QString serverName = info.at(1);
   QString serverIp = info.at(2);

   QString html = ui->tbLocalInfo->toHtml();
   html.replace(oldCharName, charName);
   html.replace(oldServerName, serverName);
   html.replace(oldServerIp, serverIp);
   ui->tbLocalInfo->setHtml(html);

   if ((oldCharName != charName) &&
      (oldCharName != LOCALINFO_CHAR_NAME_INIT))
   {
      if (ui->rbOn->isChecked())
         emit sendTs3Message(QString() + MSG_STR_RENAME_CHAR +
                     "###" + oldCharName + "###" + charName);
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

   if (ui->rbOn->isChecked())
      emit sendTs3Message(m_player.toMessage());
}

void DayzServerIp::on_rbOn_clicked()
{
   ui->pbOpenProfile->setEnabled(false);
}

void DayzServerIp::on_rbOff_clicked()
{
   ui->pbOpenProfile->setEnabled(true);
}

void DayzServerIp::on_pbRemoteInfoClear_clicked()
{
   m_remoteInfo.clear();
   setupRemoteInfo();
}

DayzServerIp::MessageType DayzServerIp::toMessage(const QStringList& infos)
{
   MessageType result = MessageType::INVALID;

   if (infos.at(0) == MSG_STR_UPDATE_SERVER)
   {
      if (infos.count() == 5)
         result = MessageType::UPDATE_SERVER;
   }
   else if (infos.at(0) == MSG_STR_RENAME_CHAR)
   {
      if (infos.count() == 3)
         result = MessageType::RENAME_CHAR;
   }

   return result;
}

void DayzServerIp::setupRemoteInfo()
{
   m_remoteInfo.setHorizontalHeaderItem(0, new QStandardItem("Player"));
   m_remoteInfo.setHorizontalHeaderItem(1, new QStandardItem("Server"));
   m_remoteInfo.setHorizontalHeaderItem(2, new QStandardItem("IP"));
   m_remoteInfo.setHorizontalHeaderItem(3, new QStandardItem("Timestamp"));
   ui->tvRemoteInfo->setModel(&m_remoteInfo);
   ui->tvRemoteInfo->setColumnWidth(0, 100);
   ui->tvRemoteInfo->setColumnWidth(1, 200);
   ui->tvRemoteInfo->setColumnWidth(2, 100);
   ui->tvRemoteInfo->setColumnWidth(3, 100);
}
