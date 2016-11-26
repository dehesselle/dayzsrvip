/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "DayzServerIp.h"
#include <QApplication>
#include "Version.h"
#include "Log.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   qInfo()  << "DayZServerIP testprogram v" << DAYZSERVERIP_VERSION;
   qInfo()  << "using Qt v" << QT_VERSION_STR;

   DayzServerIp dayzServerIp(0, "D:/Temp");
   dayzServerIp.show();

   return a.exec();
}
