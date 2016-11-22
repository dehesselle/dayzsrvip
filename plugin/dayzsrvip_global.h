#ifndef DAYZSRVIP_GLOBAL_H
#define DAYZSRVIP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DAYZSRVIP_LIBRARY)
#  define DAYZSRVIPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DAYZSRVIPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DAYZSRVIP_GLOBAL_H
