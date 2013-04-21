#ifndef QGPHOTOCAMERA_GLOBAL_H
#define QGPHOTOCAMERA_GLOBAL_H
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-list.h>
#include <QtCore/qglobal.h>

#if defined(QGPHOTOCAMERA_LIBRARY)
#  define QGPHOTOCAMERASHARED_EXPORT Q_DECL_EXPORT
#else
#  define QGPHOTOCAMERASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QGPHOTOCAMERA_GLOBAL_H
