/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"
#include "plugin.h"
//--- additional stuff required by dayzsrvip -----------------------------------
#include "DayzServerIp.h"
#include "Version.h"
#include <QString>
#include "Log.h"
//------------------------------------------------------------------------------

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 20

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

static char* pluginID = NULL;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
   int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
   *result = (char*)malloc(outlen);
   if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
      *result = NULL;
      return -1;
   }
   return 0;
}
#endif

//--- additional stuff required by dayzsrvip -----------------------------------
DayzServerIp* dayzServerIp = 0;   // main class of this plugin

enum Ts3IdProgress
{
   TIP_SERVER_CONNECTION_HANDLE = 0,
   TIP_CONNECTON_STATUS,
   TIP_CLIENT_ID,
   TIP_CHANNEL_ID,
   TIP_CLIENT_NAME
};

const char* progressToStr(Ts3IdProgress progress)
{
   switch (progress)
   {
      case TIP_SERVER_CONNECTION_HANDLE:
         return "SMP_SERVER_CONNECTION_HANDLE";
         break;
      case TIP_CONNECTON_STATUS:
         return "SMP_CONNECTON_STATUS";
         break;
      case TIP_CLIENT_ID:
         return "SMP_CLIENT_ID";
         break;
      case TIP_CHANNEL_ID:
         return "SMP_CHANNEL_ID";
         break;
      case TIP_CLIENT_NAME:
         return "SMP_CLIENT_NAME";
         break;
   }

   return "INVALID";   // just to avoid compiler warning
}

bool getTs3Ids(uint64& srvConHdlId,
               int& conStatus,
               anyID& clientId,
               uint64& channelId,
               QString& clientName,
               Ts3IdProgress& progress)
{
   bool result = false;

   progress = TIP_SERVER_CONNECTION_HANDLE;

   static const unsigned int ERROR_dummy = 999999;
   unsigned int rc = ERROR_dummy;

   srvConHdlId = ts3Functions.getCurrentServerConnectionHandlerID();
   if (srvConHdlId)   // check if connected to server
      rc = ERROR_ok;
   else
      rc = ERROR_dummy;

   if (rc == ERROR_ok)
   {
      progress = TIP_CONNECTON_STATUS;
      rc = ts3Functions.getConnectionStatus(srvConHdlId, &conStatus);

      if (rc == ERROR_ok)
         if (conStatus == 0)
            rc = ERROR_dummy;
   }

   if (rc == ERROR_ok)
   {
      progress = TIP_CLIENT_ID;
      rc = ts3Functions.getClientID(srvConHdlId, &clientId);
   }

   if (rc == ERROR_ok)
   {
      progress = TIP_CHANNEL_ID;
      rc = ts3Functions.getChannelOfClient(srvConHdlId, clientId, &channelId);
   }

   if (rc == ERROR_ok)
   {
      progress = TIP_CLIENT_NAME;
      char* name = 0;
      rc = ts3Functions.getClientVariableAsString(srvConHdlId,
                                                  clientId,
                                                  CLIENT_NICKNAME,
                                                  &name);

      if (rc == ERROR_ok)
      {
         clientName = name;
         ts3Functions.freeMemory(name);
      }
   }

   if (rc == ERROR_ok)
      result = true;

   return result;
}

void sendMessageToChannel(QString message)   // Wrapper to simplify sending
                                             // a message to current channel.
                                             // If any kind of error occurs,
                                             // the function aborts silently.
{
   uint64 srvConHdlId;
   int conStatus;
   anyID clientId;
   uint64 channelId;
   QString clientName;
   Ts3IdProgress progress;

   if (getTs3Ids(srvConHdlId, conStatus, clientId, channelId,
                 clientName, progress))
   {
      // FIXME "a little hackish": insert TS3 nickname here
      message.replace("TS3NAME_PLACEHOLDER", clientName);
      unsigned int rc = ts3Functions.requestSendChannelTextMsg(
               srvConHdlId,
               message.toStdString().c_str(),
               channelId,
               NULL);

      if (rc != ERROR_ok)
         logError("failed to send message");
   }
   else
   {
      logError(QString("failed to get ID") + progressToStr(progress));
   }
}
//------------------------------------------------------------------------------

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
   /* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
   static char* result = NULL;  /* Static variable so it's allocated only once */
   if(!result) {
      const wchar_t* name = L"DayZ Server IP";
      if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
         result = (char*)"DayZ Server IP";  /* Conversion failed, fallback here */
      }
   }
   return result;
#else
   return "DayZ Server IP";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
    return DAYZSERVERIP_VERSION;
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
   return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
   /* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "René de Hesselle";
}

/* Plugin description */
const char* ts3plugin_description() {
   /* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "This plugin distributes the server's name and IP address "
          "you're playing on to all people in your channel.\n\n"
          "https://github.com/dehesselle/dayzsrvip";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
    setTs3FunctionsForLog(funcs);
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init()
{
   int result = 0;

   logInfo(QString("initializing (version ")
           + DAYZSERVERIP_VERSION
           + QString(")"));

#ifdef DEVELOPER_MODE
   logWarning("****** DEVELOPER_MODE ******");
#endif

   char configPath[PATH_BUFSIZE];
   ts3Functions.getConfigPath(configPath, PATH_BUFSIZE);

   ::dayzServerIp = new DayzServerIp(0, configPath);

   if (::dayzServerIp)
   {
      logInfo("successfully created instance");
      ::dayzServerIp->connect(::dayzServerIp,
                              &DayzServerIp::sendTs3Message,
                              sendMessageToChannel);
      result = 0;
   }
   else
   {
      logInfo("failed to create instance");
      result = 1;   // failure with warning
   }

   return result;   /*  0 = success
                     *  1 = failure
                     * -2 = failure w/o "failed to load" warning
                     */
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown()
{
   logInfo("terminating");

   if (::dayzServerIp)
   {
      delete ::dayzServerIp;
      logInfo("terminated");
   }
   else
   {
      logInfo("nothing to terminate, most likely due to previous error");
   }

   /* Free pluginID if we registered it */
   if(pluginID)
   {
      free(pluginID);
      pluginID = 0;
   }
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
   const size_t sz = strlen(id) + 1;
   pluginID = (char*)malloc(sz * sizeof(char));
   _strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
   free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
   return 0;  /* 1 = request autoloaded, 0 = do not request autoload */
}

/* Helper function to create a menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon) {
   struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
   menuItem->type = type;
   menuItem->id = id;
   _strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
   _strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
   return menuItem;
}

/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);

/*
 * Menu IDs for this plugin. Pass these IDs when creating a menuitem to the TS3 client. When the menu item is triggered,
 * ts3plugin_onMenuItemEvent will be called passing the menu ID of the triggered menu item.
 * These IDs are freely choosable by the plugin author. It's not really needed to use an enum, it just looks prettier.
 */
enum {
   MENU_ID_GLOBAL_1 = 1
};

/*
 * Initialize plugin menus.
 * This function is called after ts3plugin_init and ts3plugin_registerPluginID. A pluginID is required for plugin menus to work.
 * Both ts3plugin_registerPluginID and ts3plugin_freeMemory must be implemented to use menus.
 * If plugin menus are not used by a plugin, do not implement this function or return NULL.
 */
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon) {
   /*
    * Create the menus
    * There are three types of menu items:
    * - PLUGIN_MENU_TYPE_CLIENT:  Client context menu
    * - PLUGIN_MENU_TYPE_CHANNEL: Channel context menu
    * - PLUGIN_MENU_TYPE_GLOBAL:  "Plugins" menu in menu bar of main window
    *
    * Menu IDs are used to identify the menu item when ts3plugin_onMenuItemEvent is called
    *
    * The menu text is required, max length is 128 characters
    *
    * The icon is optional, max length is 128 characters. When not using icons, just pass an empty string.
    * Icons are loaded from a subdirectory in the TeamSpeak client plugins folder. The subdirectory must be named like the
    * plugin filename, without dll/so/dylib suffix
    * e.g. for "test_plugin.dll", icon "1.png" is loaded from <TeamSpeak 3 Client install dir>\plugins\test_plugin\1.png
    */

   BEGIN_CREATE_MENUS(1);  /* IMPORTANT: Number of menu items must be correct! */
   CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL,  MENU_ID_GLOBAL_1,  "Show",  "");
   END_CREATE_MENUS;  /* Includes an assert checking if the number of menu items matched */

   /*
    * Specify an optional icon for the plugin. This icon is used for the plugins submenu within context and main menus
    * If unused, set menuIcon to NULL
    */
   *menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
   _strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "t.png");

   /*
    * Menus can be enabled or disabled with: ts3Functions.setPluginMenuEnabled(pluginID, menuID, 0|1);
    * Test it with plugin command: /test enablemenu <menuID> <0|1>
    * Menus are enabled by default. Please note that shown menus will not automatically enable or disable when calling this function to
    * ensure Qt menus are not modified by any thread other the UI thread. The enabled or disable state will change the next time a
    * menu is displayed.
    */
   /* For example, this would disable MENU_ID_GLOBAL_2: */
   /* ts3Functions.setPluginMenuEnabled(pluginID, MENU_ID_GLOBAL_2, 0); */

   /* All memory allocated in this function will be automatically released by the TeamSpeak client later by calling ts3plugin_freeMemory */
}

/************************** TeamSpeak callbacks ***************************/
/*
 * Following functions are optional, feel free to remove unused callbacks.
 * See the clientlib documentation for details on each function.
 */

/* Clientlib */

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID,
                                          int newStatus,
                                          unsigned int errorNumber)
{
   if (newStatus == STATUS_CONNECTION_ESTABLISHED)   // client & channels
                                                     // available
   {
      anyID clientId;

      if (ts3Functions.getClientID(
             serverConnectionHandlerID, &clientId) == ERROR_ok)
      {
         char* name;
         if (ts3Functions.getClientSelfVariableAsString(
                serverConnectionHandlerID, CLIENT_NICKNAME, &name) == ERROR_ok)
         {
            ::dayzServerIp->setTs3Name(name);
            ts3Functions.freeMemory(name);
         }
         else
         {
            ::dayzServerIp->setTs3Name("Unknown"
                                       + QString::number(qrand() % 10000));
         }
      }
      else
      {
         ::dayzServerIp->setTs3Name("Unknown"
                                    + QString::number(qrand() % 10000));
      }
   }
}

int ts3plugin_onTextMessageEvent(uint64 serverConnectionHandlerID, anyID targetMode, anyID toID, anyID fromID, const char* fromName, const char* fromUniqueIdentifier, const char* message, int ffIgnored) {
   /* Friend/Foe manager has ignored the message, so ignore here as well. */
   if(ffIgnored) {
      return 0; /* Client will ignore the message anyways, so return value here doesn't matter */
   }

   // TODO: only use messages from channel

//   uint64 srvConHdlId;
//   int conStatus;
//   anyID& clientId;
//   uint64 channelId;
//   QString clientName;
//   Ts3IdProgress progress;

//   if (getTs3Ids(srvConHdlId, conStatus, clientId, channelId,
//                 clientName, progress))
//   {

//   }

   ::dayzServerIp->onTs3MessageReceived(message);

    return 0;  /* 0 = handle normally, 1 = client will ignore the text message */
}

/* Client UI callbacks */

/*
 * Called when a plugin menu item (see ts3plugin_initMenus) is triggered. Optional function, when not using plugin menus, do not implement this.
 *
 * Parameters:
 * - serverConnectionHandlerID: ID of the current server tab
 * - type: Type of the menu (PLUGIN_MENU_TYPE_CHANNEL, PLUGIN_MENU_TYPE_CLIENT or PLUGIN_MENU_TYPE_GLOBAL)
 * - menuItemID: Id used when creating the menu item
 * - selectedItemID: Channel or Client ID in the case of PLUGIN_MENU_TYPE_CHANNEL and PLUGIN_MENU_TYPE_CLIENT. 0 for PLUGIN_MENU_TYPE_GLOBAL.
 */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID,
                               enum PluginMenuType type,
                               int menuItemID,
                               uint64 selectedItemID)
{
   switch (type)
   {
      case PLUGIN_MENU_TYPE_GLOBAL:
         // Global menu item was triggered. selectedItemID is unused and set to zero.
         switch (menuItemID)
         {
            case MENU_ID_GLOBAL_1:
               if (::dayzServerIp->isHidden())
                  ::dayzServerIp->show();
               break;
         }
         break;
      case PLUGIN_MENU_TYPE_CHANNEL:
         // Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel
         /*
         switch (menuItemID)
         {
            default:
               break;
         }
         */
         break;
      case PLUGIN_MENU_TYPE_CLIENT:
         // Client contextmenu item was triggered. selectedItemID is the clientID of the selected client
         /*
         switch (menuItemID)
         {
            default:
               break;
         }
         */
         break;
   }
}
