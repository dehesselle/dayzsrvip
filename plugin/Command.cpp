#include "Command.h"
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "Log.h"

const char* Command::XML_NAME = "dayzsrvip";
const char* Command::XML_VERSION = "version";
const char* Command::XML_VERSION_VALUE = "1";
const char* Command::XML_COMMAND = "command";
const char* Command::XML_COMMAND_SITREP = "sitrep";
const char* Command::XML_COMMAND_UPDATE = "update";

Command::Command(Type type) :
   m_player(0),
   m_type(type),
   m_wasReceived(false)
{
}

Command::Command(Player* player) :
   m_player(player),
   m_type(Type::UPDATE),
   m_wasReceived(false)
{
}

Command::~Command()
{
   if (m_wasReceived && m_player)
      delete m_player;
}

void Command::fromXmlStr(QString xmlStr)
{
   m_wasReceived = true;
   QXmlStreamReader xml(xmlStr);

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
            m_type = Type::SITREP;
         }
         else if (command == XML_COMMAND_UPDATE &&   // the order is important!
                  xml.readNext())
         {
            m_type = Type::UPDATE;
            m_player = new Player;
            m_player->fromXml(xml);
         }
      }
      xml.readNext();
   }
}

QString Command::toXmlStr(Type type)
{
   QString result;

   if (type == Type::INVALID &&
       m_type == Type::INVALID)
   {
      // This function hasn't been told what to do (argument 'type' has default
      // value 'Type::INVALID') and we haven't been able to automatically
      // determine what to do ('m_type' has value 'Type::INVALID').
      logError("Command::toXmlStr() command type not set");
   }
   else
   {
      QXmlStreamWriter xml(&result);

      xml.writeStartDocument();
      xml.writeStartElement(XML_NAME);
      xml.writeAttribute(XML_VERSION, XML_VERSION_VALUE);

      switch (m_type)
      {
         case Type::SITREP:
         {
            xml.writeAttribute(XML_COMMAND, XML_COMMAND_SITREP);
            xml.writeEndElement();
            xml.writeEndDocument();
            break;
         }
         case Type::UPDATE:
         {
            xml.writeAttribute(XML_COMMAND, XML_COMMAND_UPDATE);
            m_player->updateTimestamp();
            m_player->toXml(xml);
            xml.writeEndElement();
            xml.writeEndDocument();
            break;
         }
         case Type::INVALID:
         {
            // this should've been caught by the 'if' on top
            logError("Command::toXmlStr() Type::INVALID");
            break;
         }
      }

   }

   return result;
}

const Command::Type& Command::getType()
{
   return m_type;
}

const Player* Command::getPlayer()
{
   return m_player;
}
