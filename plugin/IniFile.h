/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#ifndef INIFILE_H
#define INIFILE_H

#include <QSettings>
#include <QString>
#include <QVariant>

/** @brief store settings with INI storage backend
 *
 * This is a wrapper-class for QSettings to handle the basic
 * parametrisation to use an INI file as storage backend.
 *
 * We're not deriving from QSettings on purpose to keep things simple.
 * (See detailed explanation in the constructor.)
 */
class IniFile
{
public:
   IniFile();
   ~IniFile();

   /// @brief name of INI setting and its default value
   ///
   /// We use KeyValue as a convenience to access a setting from an INI file as
   /// well as supplying a default value if that setting does not exist yet
   /// (e.g. on first program start with no pre-existing INI file).
   /// Best practice is to have each class that needs settings from an INI file
   /// define them as KeyValue types.
   struct KeyValue
   {
      const char *key;       ///< syntax is @c section.key
      const QString value;   ///< default value
   };

   /** @brief forwards to the respective QSettings method
    *
    * @see <a href="http://doc.qt.io/qt-5/qsettings.html#contains">
    * QSettings::contains</a>
    */
   bool contains(const QString &key) const;

   /** @brief forwards to the respective QSettings method
    * 
    * @see <a href="http://doc.qt.io/qt-5/qsettings.html#setValue">
    * QSettings::setValue</a>
    */
   void setValue(const QString &key, const QVariant &value);

   /** @brief forwards to the respective QSettings method
    * 
    * @see <a href="http://doc.qt.io/qt-5/qsettings.html#setValue">
    * QSettings::setValue</a>
    */
   void setValue(const KeyValue &keyValue, const QVariant &value = QVariant());

   /** @brief forwards to the respective QSettings method
    * 
    * @see <a href="http://doc.qt.io/qt-5/qsettings.html#value">
    * QSettings::value</a>
    */
   QVariant value(const QString &key,
                  const QVariant &defaultValue = QVariant()) const;

   /** @brief forwards to the respective QSettings method
    * 
    * @see <a href="http://doc.qt.io/qt-5/qsettings.html#value">
    * QSettings::value</a>
    */
   QVariant value(const KeyValue &keyValue);

   void openFile(const QString& filename);

private:
   QSettings *m_settings;
};

#endif // INIFILE_H
