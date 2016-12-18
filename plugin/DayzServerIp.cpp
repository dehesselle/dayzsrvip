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
#include <QMessageBox>
#include "Log.h"

const char* DayzServerIp::XML_NAME = "dayzsrvip";
const char* DayzServerIp::XML_VERSION = "version";
const char* DayzServerIp::XML_VERSION_VALUE = "1";
const char* DayzServerIp::XML_COMMAND = "command";
const char* DayzServerIp::XML_COMMAND_SITREP = "sitrep";
const char* DayzServerIp::XML_COMMAND_UPDATE = "update";

const IniFile::KeyValue DayzServerIp::INI_VERSIONNO = { "DayzServerIp/version", "" };
const IniFile::KeyValue DayzServerIp::INI_RUNCOUNT = { "DayzServerIp/runCount", "0" };
const IniFile::KeyValue DayzServerIp::INI_CHATENABLED = { "DayzServerIp/chatEnabled", "false" };
const IniFile::KeyValue DayzServerIp::INI_GEOMETRY = { "DayzServerIp/windowGeometry", "0" };

const IniFile::KeyValue DayzServerIp::INI_COL_TS3NAME =    { "DayzServerIp/columnTs3Name", "95" };
const IniFile::KeyValue DayzServerIp::INI_COL_DAYZNAME =   { "DayzServerIp/columnDayzName", "110" };
const IniFile::KeyValue DayzServerIp::INI_COL_SERVERNAME = { "DayzServerIp/columnServerName", "200" };
const IniFile::KeyValue DayzServerIp::INI_COL_SERVERIP =   { "DayzServerIp/columnServerIp", "125" };
const IniFile::KeyValue DayzServerIp::INI_COL_TIMESTAMP =  { "DayzServerIp/columnTimestamp", "80" };

DayzServerIp::DayzServerIp(QWidget *parent,
                           const QString& configPath) :
   QDialog(parent),
   ui(new Ui::DayzServerIp),
   m_scene(0),
   m_fsWatcher(new QFileSystemWatcher(this)),
   m_configPath(configPath.left(configPath.length() - 1))
{
   Q_INIT_RESOURCE(dayzsrvip);

   m_settings.openFile(m_configPath + "/dayzsrvip.ini");
   m_playerListFile = m_configPath + "/dayzsrvip.hst";

   checkVersionNo();   // this has to be called as soon as m_settings and
                       // m_playerListFile are usable

   // static UI setup
   {
      logDebug("DayzServerIp() static UI setup");

      ui->setupUi(this);

      ui->rbOff->setChecked(true);
      ui->rbOff->setToolTip("stop sending your data to everyone in your channel");
      ui->rbOn->setEnabled(false);
      ui->rbOn->setToolTip("start sending your data to everyone in your channel");

      ui->cbChat->setToolTip("send text messages to channel");

      ui->pbSitrepRequest->setEnabled(false);
      ui->pbSitrepRequest->setToolTip("request everyone to send an update");
      ui->pbRemoteInfoClear->setToolTip("clear the list");
      ui->pbProfileOpen->setToolTip("select your DayZ profile file");

      ui->gvLogo->setToolTip("DayZ is the game!");

      setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setStatusMessage(QString("Welcome! ") + DAYZSRVIP_VERSION);
   }

   // dynamic UI setup
   {
      logDebug("DayzServerIp() dynamic UI setup");

      // restore window size and position
      {
         QVariant geometry = m_settings.value(INI_GEOMETRY);

         if (geometry != "0")
            restoreGeometry(geometry.toByteArray());
      }

      setupPlayerList();   // setup columns in treeview
      ui->cbChat->setChecked(m_settings.value(INI_CHATENABLED).toBool());

      // setup HTML in tbPlayer
      {
         QFile htmlFile(":/playerTemplate");
         htmlFile.open(QFile::ReadOnly);
         QTextStream htmlStream(&htmlFile);

         while (! htmlStream.atEnd())
            m_playerHtml.append(htmlStream.readLine());

         ui->tbPlayer->setHtml(m_playerHtml);
      }

      // show DayZ Logo
      {
         m_scene = new QGraphicsScene(this);
         m_scene->addPixmap(QPixmap(":/dayzLogo"));
         ui->gvLogo->setScene(m_scene);
      }
   }

   // import history from file
   {
      // We have to open a textstream and feed QXmlStreamReader line
      // by line. We cannot use QXmlStreamReader to read the file itself
      // since it is not well-formed XML; QXmlStreamReader::atEnd is
      // already true at the end of the very first line.
      // The XML is not well-formed by design: every line is the raw output
      // from Player::toXml, there are no enclosing tags. This is due to
      // the fact that we endlessly append to the history file without wanting
      // to rewrite the whole thing each time just to make it valid XML.

      logDebug("DayzServerIp() import history");

      QFile historyFile(m_playerListFile);

      if (historyFile.open(QFile::ReadOnly))
      {
         logDebug("DayzServerIp() importing: " + historyFile.fileName());

         QTextStream historyStream(&historyFile);

         while (! historyStream.atEnd())
         {
            QString line = historyStream.readLine();
            QXmlStreamReader xml(line);
            xml.readNextStartElement();
            Player player;
            player.fromXml(xml);
            updatePlayerList(player, false);
         }
      }
      else
      {
         logDebug("DayzServerIp() no history to import");
      }
   }

   // import .DayZProfile
   {
      logDebug("DayzServerIp() import .DayZProfile");

      QString dayzProfile = m_settings.value(Player::INI_DAYZPROFILE).toString();

      if (QFile::exists(dayzProfile))
      {
         if (m_player.fromDayzProfile(dayzProfile))
         {
            updatePlayer();
            m_fsWatcher->addPath(dayzProfile);
            ui->rbOn->setEnabled(true);
         }
         // No 'else': we don't treat this as error while still constructing.
         // Although this is not really supposed to happen: a valid, working
         // profile becoming invalid.
      }
      else
      {
         logInfo("DayzServerIp() no .DayZProfile (not an error at this point)");
      }
   }

   connect(m_fsWatcher, &QFileSystemWatcher::fileChanged, this, &DayzServerIp::onFsWatcherFileChanged);
}

DayzServerIp::~DayzServerIp()
{
   m_settings.setValue(INI_GEOMETRY, saveGeometry());
   m_settings.setValue(INI_COL_TS3NAME, ui->tvPlayerList->columnWidth(PLC_TS3NAME));
   m_settings.setValue(INI_COL_DAYZNAME, ui->tvPlayerList->columnWidth(PLC_DAYZNAME));
   m_settings.setValue(INI_COL_SERVERNAME, ui->tvPlayerList->columnWidth(PLC_SERVERNAME));
   m_settings.setValue(INI_COL_SERVERIP, ui->tvPlayerList->columnWidth(PLC_SERVERIP));
   m_settings.setValue(INI_COL_TIMESTAMP, ui->tvPlayerList->columnWidth(PLC_TIMESTAMP));

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
         logDebug("updatePlayerList() new");

         int row = m_playerListModel.rowCount();

         QStandardItem* item = new QStandardItem(player.getDayzName());
         m_playerListModel.setItem(row, PLC_DAYZNAME, item);
         item = new QStandardItem(player.getTs3Name());
         m_playerListModel.setItem(row, PLC_TS3NAME, item);
         item = new QStandardItem(player.getServerName());
         m_playerListModel.setItem(row, PLC_SERVERNAME, item);
         item = new QStandardItem(player.getServerIp());
         m_playerListModel.setItem(row, PLC_SERVERIP, item);
         item = new QStandardItem(player.getTimestamp());
         m_playerListModel.setItem(row, PLC_TIMESTAMP, item);

         break;
      }
      case 1:   // update existing item
      {
         // The first row is cloned to a child row, then gets updated
         // with the data received. So the most recent update always
         // stays on top.

         logDebug("updatePlayerList() update");

         QStandardItem* firstItem = itemList.first();
         int row = firstItem->row();

         QList<QStandardItem*> items;
         for (int col = 0; col < m_playerListModel.columnCount(); col++)
            if (col)   // clone every column...
               items << m_playerListModel.item(row, col)->clone();
            else       // ...but the first one (TS3 name)
               items << new QStandardItem(QString(""));

         firstItem->insertRow(0, items);   // updates get inserted in front

         m_playerListModel.item(row, PLC_DAYZNAME)->setText(player.getDayzName());
         m_playerListModel.item(row, PLC_SERVERNAME)->setText(player.getServerName());
         m_playerListModel.item(row, PLC_SERVERIP)->setText(player.getServerIp());
         m_playerListModel.item(row, PLC_TIMESTAMP)->setText(player.getTimestamp());

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
   QString html = m_playerHtml;   // always use template-html as starting point

   html.replace(Player::INIT_TS3NAME, m_player.getTs3Name().toHtmlEscaped());
   html.replace(Player::INIT_DAYZNAME, m_player.getDayzName().toHtmlEscaped());
   html.replace(Player::INIT_SERVERNAME, m_player.getServerName().toHtmlEscaped());
   html.replace(Player::INIT_SERVERIP, m_player.getServerIp().toHtmlEscaped());
   ui->tbPlayer->setHtml(html);
}

void DayzServerIp::onFsWatcherFileChanged(const QString& path)
{
   processProfile(path);

   if (! m_fsWatcher->files().count())
      m_fsWatcher->addPath(path);
}

void DayzServerIp::on_rbOn_clicked()
{
   ui->pbProfileOpen->setEnabled(false);   // forbid changing profile while running
   ui->pbSitrepRequest->setEnabled(true);
   setStatusMessage("meddel on!");
}

void DayzServerIp::on_rbOff_clicked()
{
   ui->pbProfileOpen->setEnabled(true);   // allow changing profile
   ui->pbSitrepRequest->setEnabled(false);
   setStatusMessage("meddel off!");
}

void DayzServerIp::on_pbRemoteInfoClear_clicked()
{
   switch (QMessageBox::question(
              this,
              "clear history",
              "Do you want to clear the history from disk as well?",
              QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
              QMessageBox::Cancel))
   {
      case QMessageBox::Yes:
         if (QFile::exists(m_playerListFile))
            if (! QFile::remove(m_playerListFile))
               logError("on_pbRemoteInfoClear_clicked() failed to remove: "
                        + m_playerListFile);
         // yes, there's no 'break' here
      case QMessageBox::No:
         m_playerListModel.clear();
         setupPlayerList();
         setStatusMessage("cleared all data");
         break;
      case QMessageBox::Cancel:
         break;
   }
}

void DayzServerIp::setupPlayerList()
{
   //TODO save and restore these values in INI

   m_playerListModel.setHorizontalHeaderItem(PLC_TS3NAME, new QStandardItem("TS3 name"));
   m_playerListModel.setHorizontalHeaderItem(PLC_DAYZNAME, new QStandardItem("DayZ name"));
   m_playerListModel.setHorizontalHeaderItem(PLC_SERVERNAME, new QStandardItem("Server"));
   m_playerListModel.setHorizontalHeaderItem(PLC_SERVERIP, new QStandardItem("IP"));
   m_playerListModel.setHorizontalHeaderItem(PLC_TIMESTAMP, new QStandardItem("Timestamp"));
   ui->tvPlayerList->setModel(&m_playerListModel);
   ui->tvPlayerList->setColumnWidth(PLC_TS3NAME, m_settings.value(INI_COL_TS3NAME).toInt());
   ui->tvPlayerList->setColumnWidth(PLC_DAYZNAME, m_settings.value(INI_COL_DAYZNAME).toInt());
   ui->tvPlayerList->setColumnWidth(PLC_SERVERNAME, m_settings.value(INI_COL_SERVERNAME).toInt());
   ui->tvPlayerList->setColumnWidth(PLC_SERVERIP, m_settings.value(INI_COL_SERVERIP).toInt());
   ui->tvPlayerList->setColumnWidth(PLC_TIMESTAMP, m_settings.value(INI_COL_TIMESTAMP).toInt());
}

void DayzServerIp::onTs3CommandReceived(const QString &command)
{
   QXmlStreamReader xml(command);

   while (! xml.atEnd())
   {
      if (xml.isStartElement() &&   // the order is important!
          xml.name() == XML_NAME &&
          xml.attributes().hasAttribute(XML_VERSION) &&
          xml.attributes().value(XML_VERSION) == XML_VERSION_VALUE)
      {
         QString command(xml.attributes().value(XML_COMMAND).toString());

         if (command == XML_COMMAND_SITREP)
         {
            requestSendTs3Command(createUpdateCommand());
            setStatusMessage("Sent update to teammates.");

         }
         else if (command == XML_COMMAND_UPDATE &&   // the order is important!
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
   {
      emit sendTs3Command(command);

      if (ui->cbChat->isChecked())
         emit sendTs3Message(createUpdateMessage());
   }
   else
   {
      setStatusMessage("Not responding - still 'off'.");
   }
}

void DayzServerIp::sortPlayerList()
{
   // This way of sorting seems to only affect the child-rows of a set,
   // not the parent row. 
   m_playerListModel.sort(ui->tvPlayerList->header()->sortIndicatorSection(),
                     ui->tvPlayerList->header()->sortIndicatorOrder());
}

void DayzServerIp::savePlayerListEntry(const Player& player)
{
   QFile history(m_playerListFile);

   if (history.open(QFile::Append))
   {
      // This will produce not-well-formed XML on purpose since we want
      // to append records endlessly. (there are no enclosing tags)

      QString xmlStr;
      QXmlStreamWriter xml(&xmlStr);
      player.toXml(xml);

      QTextStream out(&history);
      out << xmlStr << "\n";
   }
   else
   {
      logError("savePlayerListEntry() failed to write: " + history.fileName());
   }
}

void DayzServerIp::checkVersionNo()   // handle plugin updates
{
   QString versionFromFile = m_settings.value(INI_VERSIONNO).toString();

   if (versionFromFile != DAYZSRVIP_VERSION)
   {
      if (QFile::exists(m_playerListFile))
      {
         // In the past, version changes often changed the history format.
         // Without any transformation routines for that, we have to discard
         // the history to not cause problems.
         // Now that we moved over to XML, this practice can probably be
         // changed in the future.

         if (QFile::remove(m_playerListFile))
         {
            logInfo("checkVersionNo() deleted history");
            m_settings.setValue(INI_VERSIONNO, DAYZSRVIP_VERSION);
         }
         else
         {
            logError("checkVersionNo() failed to remove history file "
                     + m_playerListFile);
         }
      }
      else
      {
         m_settings.setValue(INI_VERSIONNO, DAYZSRVIP_VERSION);
      }
      updateRunCount(1);   // reset counter
      logInfo("checkVersionNo() changed");
   }
   else
   {
      logDebug("checkVersionNo() unchanged");
      updateRunCount();
   }

   logInfo("checkVersionNo() runCount = " + QString::number(getRunCount()));
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
      // It's a design decision to just discard the old (and probably still
      // valid) data at this point.

      ui->rbOn->setEnabled(false);
      ui->tbPlayer->setHtml(m_playerHtml);
      m_player.initialize();
      m_fsWatcher->removePaths(m_fsWatcher->files());
   }
}

void DayzServerIp::on_pbSitrepRequest_clicked()
{
   setStatusMessage("Requesting sitrep from teammates.");
   requestSendTs3Command(createSitrepCommand());
}

void DayzServerIp::processProfile(const QString &filename,
                                  bool forceUpdate)
{
   int count = 0;

   while (count < 4)
   {
      // I can't remember why we're looping over this. Since I initially
      // wouldn't have done this without a reason (this part of the source
      // is >1 year old), I'm guessing I've run into contention problems.
      // Leaving it as it is for now.

      count++;

      if (m_player.fromDayzProfile(filename))
         break;

      QThread::sleep(1);
   }

   if (count > 3)
      logError("processProfile() failed to read DayZProfile 3 times");

   if (m_player.isChanged() || forceUpdate)
   {
      if (forceUpdate)   // sitrep forces update even if profile didn't change
         logDebug("processProfile() forcing update");

      updatePlayer();

      if (ui->rbOn->isChecked())
      {
         requestSendTs3Command(createUpdateCommand());
         setStatusMessage("Sent update to teammates.");
      }
   }
}

void DayzServerIp::updateRunCount(int count)
{
   // We count the number of times this plugin has run. The intention is
   // to automatically disable debug messages after a while.

   if (count)
      m_settings.setValue(INI_RUNCOUNT, count);
   else
      m_settings.setValue(INI_RUNCOUNT, getRunCount() + 1);

#ifndef DEVELOPER_MODE
   if (getRunCount() > 20)
      disableDebugMessages();
   else
#endif
      enableDebugMessages();
}

int DayzServerIp::getRunCount()
{
   return m_settings.value(INI_RUNCOUNT).toInt();
}

QString DayzServerIp::createUpdateCommand()
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

QString DayzServerIp::createUpdateMessage()
{
   // This will appear in the channel's chat, so it should be easily readable.

   QString result;

   result = m_player.getServerIp() + "    "
         + m_player.getServerName() + "    "
         + m_player.getDayzName();

   return result;
}

QString DayzServerIp::createSitrepCommand()
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

void DayzServerIp::on_cbChat_toggled(bool checked)
{
    m_settings.setValue(INI_CHATENABLED, checked);
}
