#ifndef QGPHOTOINTERFACE_H
#define QGPHOTOINTERFACE_H
#include <qcamerainterface.h>

#include "qgphotocamera_global.h"
#include "qgphotocamera.h"

#include <QObject>

class QGphotoInterface : public QCameraInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.ctphoto.QCameraInterface/0.1")
    Q_INTERFACES(QCameraInterface)
public:
    explicit QGphotoInterface();

    void initialize();
    void unload();
    QList<QCamera *> getcameras();
    QString name();
    QCamera * selectedCamera();

signals:
    
public slots:

private:
    GPPortInfoList         *portinfolist = NULL;
    CameraAbilitiesList    *abilities = NULL;
    GPContext *context;
    QList<QCamera *> _cameras;
    QList<Camera *> _gcams;
    QGphotoCamera *open_camera( const char *model, const char *port);
};

#endif // QGPHOTOINTERFACE_H
