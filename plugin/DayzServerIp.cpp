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
#include <QList>
#include <QThread>
#include <QDateTime>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include "Log.h"

const char* DayzServerIp::PLAYER_SERVERNAME_INIT = "___SERVER_NAME___";
const char* DayzServerIp::PLAYER_SERVERIP_INIT = "___SERVER_IP___";
const char* DayzServerIp::PLAYER_INGAMENAME_INIT = "___CHAR_NAME___";
const char* DayzServerIp::PLAYER_TS3NAME_INIT = "___TS3_NAME___";

const char* DayzServerIp::XML_NAME = "dayzsrvip";
const char* DayzServerIp::XML_VERSION = "version";
const char* DayzServerIp::XML_VERSION_VALUE = "1";
const char* DayzServerIp::XML_COMMAND = "command";
const char* DayzServerIp::XML_COMMAND_SITREP = "sitrep";
const char* DayzServerIp::XML_COMMAND_UPDATE = "update";

const IniFile::KeyValue DayzServerIp::INI_VERSION_NO = { "DayzServerIp/version", "" };
const IniFile::KeyValue DayzServerIp::INI_RUN_COUNT = { "DayzServerIp/runCount", "0" };

DayzServerIp::DayzServerIp(QWidget *parent,
                           const QString& configPath) :
   QDialog(parent),
   ui(new Ui::DayzServerIp),
   m_scene(0),
   m_fsWatcher(new QFileSystemWatcher(this)),
   m_configPath(configPath.left(configPath.length() - 1))
{
   Q_INIT_RESOURCE(dayzsrvip);

   // UI setup
   {
      ui->setupUi(this);

      ui->rbOff->setChecked(true);
      ui->rbOff->setToolTip("stop sending your data to everyone in your channel");
      ui->rbOn->setEnabled(false);
      ui->rbOn->setToolTip("start sending your data to everyone in your channel");

      ui->pbSitrepRequest->setEnabled(false);
      ui->pbSitrepRequest->setToolTip("request everyone to send an update");
      ui->pbRemoteInfoClear->setToolTip("clear the list");
      ui->pbProfileOpen->setToolTip("select your DayZ profile file");
      ui->pbLogOpen->setToolTip("open TeamSpeak log");

      ui->gvLogo->setToolTip("DayZ is the game!");

      setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setStatusMessage(QString("Welcome! ") + DAYZSERVERIP_VERSION);
   }

   logDebug("m_settings.openFile()");
   m_settings.openFile(m_configPath + "/dayzsrvip.ini");
   m_playerListFile = m_configPath + "/dayzsrvip.hst";
   checkVersionNo();   // this has to called ASAP to handle version changes

   // show DayZ Logo
   {
      m_scene = new QGraphicsScene(this);
      m_scene->addPixmap(QPixmap(":/dayzlogo"));
      ui->gvLogo->setScene(m_scene);
   }

   logDebug("setupRemoteInfo()");
   setupPlayerList();   // setup columns in treeview

   // import history from file
   {
      QFile history(m_playerListFile);

      if (history.open(QFile::ReadOnly))
      {
         logDebug("importing history from: " + history.fileName());

         QXmlStreamReader xml(&history);

         while (! xml.atEnd() &&
                xml.readNextStartElement())
         {
            Player player;
            player.fromXml(xml);
            updatePlayerList(player, false);
         }
      }
      else
      {
         logDebug("no history to import");
      }
   }

   m_player.setTs3Name(PLAYER_TS3NAME_INIT);

   // import .DayZProfile
   {
      QString dayzProfile = m_settings.value(Player::INI_DAYZPROFILE).toString();

      if (QFile::exists(dayzProfile))
      {
         if (m_player.fromDayzProfile(dayzProfile))
         {
            updatePlayer();
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

   connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &DayzServerIp::onFsWatcherFileChanged);
}

DayzServerIp::~DayzServerIp()
{
   delete ui;
}

void DayzServerIp::setTs3Name(const QString& name)
{
   m_player.setTs3Name(name);
   updatePlayer();
}

void DayzServerIp::updatePlayerList(const Player& player,
                                    bool saveToFile)
{
   QList<QStandardItem*> itemList = m_playerListModel.findItems(player.getTs3Name());

   switch (itemList.count())
   {
      case 0:   // insert new item
      {
         logDebug("updateRemoteInfo: new");

         int row = m_playerListModel.rowCount();

         QStandardItem* item = new QStandardItem(player.getDayzName());
         m_playerListModel.setItem(row, RIC_INGAME_NAME, item);
         item = new QStandardItem(player.getTs3Name());
         m_playerListModel.setItem(row, RIC_TS3_NAME, item);
         item = new QStandardItem(player.getServerName());
         m_playerListModel.setItem(row, RIC_SERVER_NAME, item);
         item = new QStandardItem(player.getServerIp());
         m_playerListModel.setItem(row, RIC_SERVER_IP, item);
         item = new QStandardItem(player.getTimestamp());
         m_playerListModel.setItem(row, RIC_TIMESTAMP, item);

         break;
      }
      case 1:   // update existing item
      {
         logDebug("updateRemoteInfo: update");

         QStandardItem* firstItem = itemList.at(RIC_TS3_NAME);
         int row = firstItem->row();

         QList<QStandardItem*> items;
         for (int col = 0; col < m_playerListModel.columnCount(); col++)
            if (col)
               items << m_playerListModel.item(row, col)->clone();
            else   // first column (TS3 name) doesn't get cloned
               items << new QStandardItem(QString(""));

         firstItem->insertRow(0, items);

         m_playerListModel.item(row, RIC_INGAME_NAME)->setText(player.getDayzName());
         m_playerListModel.item(row, RIC_SERVER_NAME)->setText(player.getServerName());
         m_playerListModel.item(row, RIC_SERVER_IP)->setText(player.getServerIp());
         m_playerListModel.item(row, RIC_TIMESTAMP)->setText(player.getTimestamp());

         break;
      }
      default:
         break;
   }

   sortPlayerList();
   if (saveToFile)
      savePlayerListEntry(player);
}

void DayzServerIp::updatePlayer()
{
   // Why do we keep extra variables here for old data instead using
   // Player::m_oldData? - Design decision! This is GUI-specific and
   // Player is not supposed to care about GUI-specifics.
   static QString oldServerName = PLAYER_SERVERNAME_INIT;
   static QString oldServerIp = PLAYER_SERVERIP_INIT;
   static QString oldCharName = PLAYER_INGAMENAME_INIT;
   static QString oldTs3Name = PLAYER_TS3NAME_INIT;

   QString html = ui->tbPlayer->toHtml();
   html.replace(oldTs3Name, m_player.getTs3Name().toHtmlEscaped());
   html.replace(oldCharName, m_player.getDayzName().toHtmlEscaped());
   html.replace(oldServerName, m_player.getServerName().toHtmlEscaped());
   html.replace(oldServerIp, m_player.getServerIp().toHtmlEscaped());
   ui->tbPlayer->setHtml(html);

   oldCharName = m_player.getDayzName().toHtmlEscaped();
   oldServerName = m_player.getServerName().toHtmlEscaped();
   oldServerIp = m_player.getServerIp().toHtmlEscaped();
   oldTs3Name = m_player.getTs3Name().toHtmlEscaped();
}

void DayzServerIp::onFsWatcherFileChanged(const QString& path)
{
   processProfile(path);

   if (! m_fsWatcher->files().count())
      m_fsWatcher->addPath(path);
}

void DayzServerIp::on_rbOn_clicked()
{
   ui->pbProfileOpen->setEnabled(false);
   ui->pbSitrepRequest->setEnabled(true);
   setStatusMessage("meddel on!");
}

void DayzServerIp::on_rbOff_clicked()
{
   ui->pbProfileOpen->setEnabled(true);
   ui->pbSitrepRequest->setEnabled(false);
   setStatusMessage("meddel off!");
}

void DayzServerIp::on_pbRemoteInfoClear_clicked()
{
   if (! QFile::remove(m_playerListFile))
      logError("failed to remove history file: " + m_playerListFile);

   m_playerListModel.clear();
   setupPlayerList();
   setStatusMessage("cleared all data");
}

void DayzServerIp::setupPlayerList()
{
   m_playerListModel.setHorizontalHeaderItem(RIC_TS3_NAME, new QStandardItem("TS3 name"));
   m_playerListModel.setHorizontalHeaderItem(RIC_INGAME_NAME, new QStandardItem("ingame name"));
   m_playerListModel.setHorizontalHeaderItem(RIC_SERVER_NAME, new QStandardItem("Server"));
   m_playerListModel.setHorizontalHeaderItem(RIC_SERVER_IP, new QStandardItem("IP"));
   m_playerListModel.setHorizontalHeaderItem(RIC_TIMESTAMP, new QStandardItem("Timestamp"));
   ui->tvPlayerList->setModel(&m_playerListModel);
   ui->tvPlayerList->setColumnWidth(RIC_TS3_NAME, 95);
   ui->tvPlayerList->setColumnWidth(RIC_INGAME_NAME, 110);
   ui->tvPlayerList->setColumnWidth(RIC_SERVER_NAME, 200);
   ui->tvPlayerList->setColumnWidth(RIC_SERVER_IP, 125);
   ui->tvPlayerList->setColumnWidth(RIC_TIMESTAMP, 80);
}

void DayzServerIp::onTs3CommandReceived(const QString &command)
{
   QXmlStreamReader xml(command);

   while (! xml.atEnd())
   {
      if (xml.isStartElement() &&
          xml.name() == XML_NAME &&
          xml.attributes().hasAttribute(XML_VERSION) &&
          xml.attributes().value(XML_VERSION) == XML_VERSION_VALUE)
      {
         QString command(xml.attributes().value(XML_COMMAND).toString());

         if (command == XML_COMMAND_SITREP)
         {
            requestSendTs3Command(createCommandUpdate());
            setStatusMessage("Sent update to teammates.");

         }
         else if (command == XML_COMMAND_UPDATE &&
                  xml.readNext())
         {
            Player player;
            player.fromXml(xml);
            updatePlayerList(player, true);
         }
      }
      xml.readNext();
   }
}

void DayzServerIp::setStatusMessage(const QString &message)
{
   ui->lMessage->setText(QDateTime::currentDateTime().toString("[hh:mm:ss] ") + message);
}

void DayzServerIp::requestSendTs3Command(const QString& command)
{
   if (ui->rbOn->isChecked())
      emit sendTs3Command(command);
   else
      setStatusMessage("Not responding - still 'off'.");
}

void DayzServerIp::requestSendTs3Message(const QString &message)
{
   if (ui->rbOn->isChecked())
      emit sendTs3Message(message);
   else
      setStatusMessage("Not responding - still 'off'.");
}

void DayzServerIp::sortPlayerList()
{
   m_playerListModel.sort(ui->tvPlayerList->header()->sortIndicatorSection(),
                     ui->tvPlayerList->header()->sortIndicatorOrder());
}

void DayzServerIp::savePlayerListEntry(const Player& player)
{
   QFile history(m_playerListFile);

   if (history.open(QFile::Append))
   {
      QString xmlStr;
      QXmlStreamWriter xml(&xmlStr);
      player.toXml(xml);

      QTextStream out(&history);
      out << xmlStr << "\n";
   }
   else
   {
      logError("failed to write history: " + history.fileName());
   }
}

void DayzServerIp::checkVersionNo()   // handle plugin updates
{
   QString versionFromFile = m_settings.value(INI_VERSION_NO).toString();

   if (versionFromFile != DAYZSERVERIP_VERSION)
   {
      if (QFile::exists(m_playerListFile))
      {
         if (QFile::remove(m_playerListFile))
         {
            logInfo("version change: deleted history");
            m_settings.setValue(INI_VERSION_NO, DAYZSERVERIP_VERSION);
         }
         else
         {
            logError("version change: failed to remove history file "
                     + m_playerListFile);
         }
      }
      else
      {
         m_settings.setValue(INI_VERSION_NO, DAYZSERVERIP_VERSION);
      }
      updateRunCount(1);   // reset counter
   }
   else
   {
      logDebug("version ok");
      updateRunCount();
   }

   logInfo("runCount = " + m_settings.value(INI_RUN_COUNT).toString());
}

void DayzServerIp::on_pbLogOpen_clicked()
{
   QDir logDir(m_configPath + "/logs");
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

void DayzServerIp::on_pbProfileOpen_clicked()
{
   QString dayzProfile =
         QFileDialog::getOpenFileName(
            this,
            "Open DayZ Profile",
            QStandardPaths::locate(QStandardPaths::DocumentsLocation,
                                   QString(),
                                   QStandardPaths::LocateDirectory) + "/DayZ",
            "DayZ Profile (*.DayZProfile)");

   if (QFile::exists(dayzProfile))
   {
      if (m_player.fromDayzProfile(dayzProfile))
      {
         m_settings.setValue(Player::INI_DAYZPROFILE, dayzProfile);

         if (m_player.isChanged())
            updatePlayer();

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

void DayzServerIp::on_pbSitrepRequest_clicked()
{
   setStatusMessage("Requesting sitrep from teammates.");
   requestSendTs3Command(createCommandSitrep());
}

void DayzServerIp::processProfile(const QString &filename,
                                  bool forceUpdate)
{
   int count = 0;

   while (count < 4)
   {
      // I can't remember why I'm looping over this. Since I initially
      // won't have done this without reason (this part of the source
      // is >1 year old), I'm guessing I've run into
      // contention problems. Leaving it as it is for now.

      count++;

      if (m_player.fromDayzProfile(filename))
         break;

      QThread::sleep(1);
   }

   if (count > 3)
      logError("unable to import DayZProfile after 3 attempts");

   if (m_player.isChanged() || forceUpdate)
   {
      if (forceUpdate)
         logDebug("forcing update");

      updatePlayer();

      if (ui->rbOn->isChecked())
      {
         requestSendTs3Command(createCommandUpdate());
         setStatusMessage("Sent update to teammates.");
      }
   }
}

void DayzServerIp::updateRunCount(int count)
{
   if (count)
      m_settings.setValue(INI_RUN_COUNT, count);
   else
      m_settings.setValue(INI_RUN_COUNT,
                          m_settings.value(INI_RUN_COUNT).toInt() + 1);
}

QString DayzServerIp::createCommandUpdate()
{
   QString result;

   QXmlStreamWriter xml(&result);
   xml.writeStartDocument();
   xml.writeStartElement(XML_NAME);
   xml.writeAttribute(XML_VERSION, XML_VERSION_VALUE);
   xml.writeAttribute(XML_COMMAND, XML_COMMAND_UPDATE);
   m_player.updateTimestamp();
   m_player.toXml(xml);
   xml.writeEndElement();
   xml.writeEndDocument();

   return result;
}

QString DayzServerIp::createCommandSitrep()
{
   QString result;

   QXmlStreamWriter xml(&result);
   xml.writeStartDocument();
   xml.writeStartElement(XML_NAME);
   xml.writeAttribute(XML_VERSION, XML_VERSION_VALUE);
   xml.writeAttribute(XML_COMMAND, XML_COMMAND_SITREP);
   xml.writeEndElement();
   xml.writeEndDocument();

   return result;
}
