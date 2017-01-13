#ifndef COMMAND_H
#define COMMAND_H

#include <QString>
#include "Player.h"

class Command
{
public:
   static const char* XML_NAME;             ///< tag
   static const char* XML_VERSION;          ///< attribute name
   static const char* XML_VERSION_VALUE;    ///< attribute value
   static const char* XML_COMMAND;          ///< attribute name
   static const char* XML_COMMAND_SITREP;   ///< attribute value
   static const char* XML_COMMAND_UPDATE;   ///< attribute value

   enum class Type
   {
      INVALID = 0,   ///< initial value
      UPDATE,        ///< player data
      SITREP         ///< sitrep request
   };

   Command(Type type = Type::INVALID);
   Command(Player* player);

   ~Command();

   void fromXmlStr(QString xmlStr);
   QString toXmlStr(Type type = Type::INVALID);

   const Player* getPlayer();
   const Type& getType();

private:
   Player* m_player;
   Type m_type;
   bool m_wasReceived;

};

#endif // COMMAND_H
