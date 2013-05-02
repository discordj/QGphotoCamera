#ifndef QGPHOTOCAMERA_H
#define QGPHOTOCAMERA_H

#include <qcamera.h>
#include <QRegExp>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QBuffer>
#include <QMutex>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include "qgphotocamera_global.h"

class QGPHOTOCAMERASHARED_EXPORT QGphotoCamera : public QCamera
{
    Q_OBJECT

public:
    QGphotoCamera();

    int QCConnect();
    int QCDisconnect(){ return 1;}

    void capture(int seconds=0);
    void setbulbmode(bool bulb){}
    void startbulbexposure(){}
    void stopbulbexposure(){}
    QImage getImage();
    QString getImageFile();
    QString identifier() {return QString("%1 (%2)").arg(_model).arg(_identifier); }
    QString model() { return _model;}

    void setSelected() {}
    QCameraProperties *getCameraProperties();
    QCameraProperty *getCameraProperty(QCameraProperties::QCameraPropertyTypes prop);
    QCameraProperties *getallproperties();
    void setCameraProperty(QCameraProperties::QCameraPropertyTypes prop, QVariant value);

    void setImageDirectory(QString dir){_imagedestdir = dir;}
    void setImageFilePrefix(QString imagenameprefix){_nameprefix = imagenameprefix;}


    void toggleLiveView(bool onoff);

    int batteryLevel();

    bool hasBulbMode(){if(_gpsettings["hasBulb"] == "true") return true; else return false; }
    virtual bool canSetBulbMode(){return false; }
    virtual bool hasLiveView(){if(_gpsettings["hasLiveView"] == "true") return true; else return false;}
    virtual bool canStreamLiveView() {return true; }
    void initializeLiveView();
    QPixmap *getLiveViewImage();
    void endLiveView();



    virtual void lockUI(){}
    virtual void unlockUI(){}

    Camera *get_gpcamera(){ return _gpcamera; }
    void set_gpcamera(Camera *cam) { _gpcamera = cam; }
    GPContext *get_gpcontext(){ return _gpcontext; }
    void set_gpcontext(GPContext *context) { _gpcontext = context; }
    void set_model(QString model){ _model = model; }
    void set_identifier(QString identifier) { _identifier = identifier; }



protected:
    virtual void notifypropertychanged(QCameraProperties::QCameraPropertyTypes prop, QVariant value){emit camera_property_changed(prop, value);}

private:
    Camera *_gpcamera;
    GPContext *_gpcontext;
    QString _model;
    QString _identifier;
    QCameraProperties *_properties;
    QString _imagedestdir;
    QString _nameprefix;
    int _imagecount;
    int _lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child);
    QCameraProperty *gpconfigtocameraproperty(QString propName, char *config);

    QTimer *_lvTimer;
    bool _liveViewReady;

    QMutex _lvMutex;

    QMap<QString,QString> _gpsettings;
    QList<QString> _imageFiles;

    void getSettingsInfo();
    void initializeCanonLV();
    void endCanonLV();
};

#endif // QGPHOTOCAMERA_H
