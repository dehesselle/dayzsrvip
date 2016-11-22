/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "DayzServerIp.h"
#include <QApplication>
#include <QString>
#include <QFileInfo>
#include "Version.h"
#include "Log.h"

INITIALIZE_EASYLOGGINGPP

// order represents log hierarchy
static const IniFile::KeyValue INI_LOG_TRACE = { "main/logTrace", "false" };
static const IniFile::KeyValue INI_LOG_DEBUG = { "main/logDebug", "false" };
static const IniFile::KeyValue INI_LOG_VERBOSE = { "main/logVerbose", "false" };
static const IniFile::KeyValue INI_LOG_INFO = { "main/logInfo", "true" };
static const IniFile::KeyValue INI_LOG_WARNING = { "main/logWarning", "true" };
static const IniFile::KeyValue INI_LOG_ERROR = { "main/logError", "true" };
static const IniFile::KeyValue INI_LOG_FATAL = { "main/logFatal", "true" };
static const IniFile::KeyValue INI_LOG_FORMAT = {
   "main/logFormat", "%datetime %level [%func] %msg"
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   // setup logging
   {
      QString logfile =
            "D:/Temp/" +
            QFileInfo(QCoreApplication::applicationFilePath()).baseName() +
            ".log";

      IniFile settings;
      settings.openFile("D:/Temp/" +
                        QFileInfo(QCoreApplication::applicationFilePath()).baseName() +
                        ".ini");

      el::Configurations config;
      config.setToDefault();

      config.setGlobally(el::ConfigurationType::Filename,
                         logfile.toStdString());
      config.setGlobally(el::ConfigurationType::Format,
            settings.value(INI_LOG_FORMAT).toString().toStdString());
      config.setGlobally(el::ConfigurationType::ToFile, "true");

      config.set(el::Level::Trace, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_TRACE).toString().toStdString());
      config.set(el::Level::Debug, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_DEBUG).toString().toStdString());
      config.set(el::Level::Verbose, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_VERBOSE).toString().toStdString());
      config.set(el::Level::Info, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_INFO).toString().toStdString());
      config.set(el::Level::Warning, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_WARNING).toString().toStdString());
      config.set(el::Level::Error, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_ERROR).toString().toStdString());
      config.set(el::Level::Fatal, el::ConfigurationType::Enabled,
                 settings.value(INI_LOG_FATAL).toString().toStdString());

#ifdef QT_DEBUG
      // console logging (in addition to file) only in debug builds
      config.setGlobally(el::ConfigurationType::ToStandardOutput, "true");
      // force log levels in debug builds
      config.set(el::Level::Global, el::ConfigurationType::Enabled, "true");
#else
      config.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
#endif

      el::Loggers::reconfigureAllLoggers(config);
   }

   LOG(INFO)  << "DayZServerIP testprogram v" << DAYZSERVERIP_VERSION;
   LOG(DEBUG) << "using Easylogging++ v" << el::VersionInfo::version();
   LOG(DEBUG) << "using Qt v" << QT_VERSION_STR;

   TIMED_FUNC(timerObj);

   DayzServerIp dayzServerIp(0, "D:/Temp");
   dayzServerIp.show();

   return a.exec();
}
