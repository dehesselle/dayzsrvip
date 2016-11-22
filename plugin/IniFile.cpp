/*
 * DayZ Server IP - TeamSpeak 3 plugin
 *
 * https://github.com/dehesselle/dayzsrvip
 */

#include "IniFile.h"

IniFile::IniFile() :
   m_settings(0)
{
}

IniFile::~IniFile()
{
   if (m_settings)
      delete m_settings;
}

bool IniFile::contains(const QString &key) const
{
   return m_settings->contains(key);
}

void IniFile::setValue(const QString &key, const QVariant &value)
{
   m_settings->setValue(key, value);
}

QVariant IniFile::value(const QString &key, const QVariant &defaultValue) const
{
   return m_settings->value(key, defaultValue);
}

QVariant IniFile::value(const IniFile::KeyValue &keyValue)
{
   if (! m_settings->contains(keyValue.key))
      m_settings->setValue(keyValue.key, keyValue.value);

   return m_settings->value(keyValue.key);
}

void IniFile::setValue(const IniFile::KeyValue &keyValue, const QVariant &value)
{
   if (value.isNull())
      m_settings->setValue(keyValue.key, keyValue.value);
   else
      m_settings->setValue(keyValue.key, value);
}

void IniFile::openFile(const QString& filename)
{
   if (m_settings)
      delete m_settings;

   m_settings = new QSettings(filename, QSettings::IniFormat);
}
