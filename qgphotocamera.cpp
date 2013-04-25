#include "dcrimage.h"
#include "qgphotocamera.h"


QGphotoCamera::QGphotoCamera()
{
    _properties = NULL;
    _imagecount = 0;
    _imagedestdir = QDir::currentPath();
}

int QGphotoCamera::QCConnect(){
    getSettingsInfo();
    getCameraProperties();
    return 1;
}

QCameraProperty *QGphotoCamera::getCameraProperty(QCameraProperties::QCameraPropertyTypes prop){
qDebug("Returning property");
    return _properties->getCameraProperty(prop);
}

QCameraProperties *QGphotoCamera::getCameraProperties(){
    if(_properties) {
        delete _properties;
        _properties = NULL;
    }
    _properties = getallproperties();

    return _properties;
}

QCameraProperties *QGphotoCamera::getallproperties(){
    QCameraProperties *props = new QCameraProperties();

    QCameraProperty *camProp = gpconfigtocameraproperty(QString("Resolutions"), _gpsettings["imagequality"].toLatin1().data());


    props->addProperty(QCameraProperties::ResolutionMode, camProp);

    camProp = gpconfigtocameraproperty(QString("Aperture"), _gpsettings["aperture"].toLatin1().data());


    props->addProperty(QCameraProperties::Aperture, camProp);

    camProp = gpconfigtocameraproperty(QString("Exposures"), _gpsettings["shutterspeed"].toLatin1().data());
    props->addProperty(QCameraProperties::ExposureTimes, camProp);

    camProp = gpconfigtocameraproperty(QString("ExposuresWhiteBalance"), _gpsettings["whitebalance"].toLatin1().data());
    props->addProperty(QCameraProperties::WhiteBalanceMode, camProp);

    camProp = gpconfigtocameraproperty(QString("Iso"), _gpsettings["iso"].toLatin1().data());
    props->addProperty(QCameraProperties::Iso, camProp);

    return props;

}

QCameraProperty *QGphotoCamera::gpconfigtocameraproperty(QString propName, char *config){
    QCameraProperty *camProp = new QCameraProperty(propName);
    qDebug(qPrintable(QString("Getting config for %0").arg(config)));
   CameraWidget *widget = NULL;
    int ret = gp_camera_get_config (_gpcamera, &widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_get_config failed: %0\n").arg(ret)));
          return 0;
    }
    char *label = NULL;


    CameraWidget *child = NULL;
    CameraWidgetType type;

    ret = _lookup_widget(widget, config, & child);
    if(ret < GP_OK){
        return 0;
    }
    gp_widget_get_type(child,&type);
    char  *val;
    ret = gp_widget_get_value(child, &val);
    if( ret == GP_OK){
        camProp->setCurrentValue(val);

        qDebug(qPrintable(QString("Current Value is %0").arg(val)));
    }

    ret = gp_widget_count_choices(child);
    for(int i=0; i < ret; i++){
        int cret = gp_widget_get_choice(child, i, (const char **)&label);
//        if(strcmp(label, val) == 0){
//                camProp->setCurrentValue(i);
//            }
        if(cret = GP_OK){
            qDebug(qPrintable(QString("choice is %0").arg(label)));
            camProp->appendValue(QString(label),QString(label));
        }
    }


    gp_widget_free(widget);
    return camProp;
}

void QGphotoCamera::setCameraProperty(QCameraProperties::QCameraPropertyTypes prop, QVariant value){
    CameraWidget *widget = NULL;
    int ret = gp_camera_get_config (_gpcamera, &widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_get_config failed: %0\n").arg(ret)));
          return;
    }


    CameraWidget *child = NULL;
    CameraWidgetType type;

    switch(prop){
        case QCameraProperties::ResolutionMode:
        ret = _lookup_widget(widget, _gpsettings["imagequality"].toLatin1().data(), & child);

        break;
        case QCameraProperties::Aperture:
                ret = _lookup_widget(widget, _gpsettings["aperture"].toLatin1().data(), &child);
                break;
        case QCameraProperties::ExposureTimes:
            ret = _lookup_widget(widget, _gpsettings["shutterspeed"].toLatin1().data(), &child);
            break;
        case QCameraProperties::WhiteBalanceMode:
            ret = _lookup_widget(widget, _gpsettings["whitebalance"].toLatin1().data(), &child);
            break;
        case QCameraProperties::Iso:
            ret=_lookup_widget(widget, _gpsettings["iso"].toLatin1().data(), &child);
            break;
    }

    if(ret < GP_OK){
        qDebug(qPrintable(QString("Couldn't find setting: %0").arg(ret)));
        return;
        }
    char *val = value.toString().toLatin1().data();

    ret = gp_widget_set_value (child, val);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("could not set widget value: %0\n").arg(gp_result_as_string(ret))));
          return;
    }
    /* This stores it on the camera again */
    ret = gp_camera_set_config (_gpcamera, widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_set_config failed: %0\n").arg(ret)));
          return;
    }

    gp_widget_free(widget);
}
int QGphotoCamera::batteryLevel(){
    CameraWidget *widget = NULL;
     int ret = gp_camera_get_config (_gpcamera, &widget, _gpcontext);
     if (ret < GP_OK) {
           qDebug(qPrintable(QString("camera_get_config failed: %0\n").arg(ret)));
           return 0;
     }


     CameraWidget *child = NULL;
     CameraWidgetType type;

     ret = _lookup_widget(widget, "batterylevel", & child);
     if(ret < GP_OK){
         return 0;
     }

     char  *val;
     ret = gp_widget_get_value(child, &val);
    QString level(val);

    level.replace(QRegExp("[^0-9]"),"");
    qDebug(qPrintable(QString("Level: %0").arg(level)));

    int ival = level.toUInt();
    qDebug(qPrintable(QString("Battery level at: %0").arg(ival)));
    return ival;
}

void QGphotoCamera::capture(int seconds){

    QString filename;
    CameraFile *file;
     CameraFilePath camera_file_path;
    CameraFileType ftype;
    bool isRaw = false;
    int retval;

    if(_liveViewReady) endLiveView();
     // this was done in the libphoto2 example code, but doesn't seem to be necessary
     // NOP: This gets overridden in the library to /capt0000.jpg
     //snprintf(camera_file_path.folder, 1024, "/");
     //snprintf(camera_file_path.name, 128, "foo.jpg");

     // take a shot
     retval = gp_camera_capture(_gpcamera, GP_CAPTURE_IMAGE, &camera_file_path, _gpcontext);

     if (retval < GP_OK) {
      // do some error handling, probably return from this function
    return;
     }


     printf("Pathname on the camera: %s/%s\n", camera_file_path.folder, camera_file_path.name);

    QString origfilename(camera_file_path.name);

    QStringList fileparts = origfilename.split('.');
        ftype = GP_FILE_TYPE_NORMAL;
    if(fileparts[1].compare(".jpg") != 0)
        isRaw = true;

     filename = QString("%1/%2_%4.%3").arg(_imagedestdir).arg(_nameprefix).arg(fileparts[1]).arg(_imagecount,4,10,QChar('0'));


     QFile fd(filename);

    if(!fd.open(QIODevice::ReadWrite)){
        return;
    }
     // create new CameraFile object from a file descriptor
     retval = gp_file_new_from_fd(&file, fd.handle());

     if (retval) {
      // error handling
         return;
     }

     // copy picture from camera
     retval = gp_camera_file_get(_gpcamera, camera_file_path.folder, camera_file_path.name, ftype, file, _gpcontext);

     if (retval) {
      // error handling
         return;
     }

     // remove picture from camera memory
     retval = gp_camera_file_delete(_gpcamera, camera_file_path.folder, camera_file_path.name, _gpcontext);

     if (retval) {
      // error handling

     }

     // free CameraFile object
     gp_file_free(file);

     // Code from here waits for camera to complete everything.
     // Trying to take two successive captures without waiting
     // will result in the camera getting randomly stuck.
     int waittime = 2000;
     CameraEventType type;
     void *data;

     printf("Wait for events from camera\n");
     while(1) {
      retval = gp_camera_wait_for_event(_gpcamera, waittime, &type, &data, _gpcontext);
      if(type == GP_EVENT_TIMEOUT) {
       break;
      }
      else if (type == GP_EVENT_CAPTURE_COMPLETE) {
       printf("Capture completed\n");
       waittime = 100;
      }
      else if (type != GP_EVENT_UNKNOWN) {
       printf("Unexpected event received from camera: %d\n", (int)type);
      }
     }


     QImage image;
     if(isRaw)
     {
         //Do raw processing
         DcRImage dcraw;
         qDebug("Conducting Raw Processing");
         //incase the of long shutter exposure and camera hasn't finished writing and it grabs the pic before and it turns out to be jpg
         if(dcraw.isRaw(filename)){
             //if(_usedarkframe && QFile::exists(_darkframe))
             //{
             //	QStringList args;
             //	args += "dcrawqt";
             //	args += "-T";
             //	args += "-c";
             //	args += QString("-K %1").arg(_darkframe);

             //	dcraw.load(filename, args);
             //}
             //else
                 dcraw.loadthumbnail(filename);

             //QByteArray *image =dcraw.GetImage(previewFile.absoluteFilePath());

             image = dcraw.getthumbimage(); //.loadFromData(*image);
         }
        else{
            qDebug("File isn't Raw!?");
        }
     }
     else
     {
         image.load(filename);
     }


     emit image_captured(image);

     if(_liveViewReady) initializeLiveView();
}


void QGphotoCamera::toggleLiveView(bool onoff){
    if(onoff){
        initializeLiveView();
        _liveViewReady = true;
    }
    else{
        endLiveView();
        _liveViewReady = false;
    }
}


void QGphotoCamera::initializeLiveView()
{
    if(_gpsettings["lvType"] == "canon"){
        initializeCanonLV();
    }
}
void QGphotoCamera::endLiveView()
{
    if(_gpsettings["lvType"] == "canon"){
        endCanonLV();
    }
}


void QGphotoCamera::initializeCanonLV(){
    CameraWidget *widget = NULL;
    int ret = gp_camera_get_config (_gpcamera, &widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_get_config failed: %0\n").arg(ret)));
          return;
    }


    CameraWidget *child = NULL;
    CameraWidgetType type;

            ret=_lookup_widget(widget, "output", &child);


    if(ret < GP_OK){
        qDebug(qPrintable(QString("Couldn't find setting: %0").arg(ret)));
        return;
        }
    QString value("PC");
    char *val = value.toLatin1().data();

    ret = gp_widget_set_value (child, val);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("could not set widget value: %0\n").arg(gp_result_as_string(ret))));
          return;
    }
    /* This stores it on the camera again */
    ret = gp_camera_set_config (_gpcamera, widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_set_config failed: %0\n").arg(ret)));
          return;
    }

    gp_widget_free(widget);
}

void QGphotoCamera::endCanonLV(){
    CameraWidget *widget = NULL;
    int ret = gp_camera_get_config (_gpcamera, &widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_get_config failed: %0\n").arg(ret)));
          return;
    }


    CameraWidget *child = NULL;
    CameraWidgetType type;

            ret=_lookup_widget(widget, "output", &child);


    if(ret < GP_OK){
        qDebug(qPrintable(QString("Couldn't find setting: %0").arg(ret)));
        return;
        }
    QString value("TFT");
    char *val = value.toLatin1().data();

    ret = gp_widget_set_value (child, val);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("could not set widget value: %0\n").arg(gp_result_as_string(ret))));
          return;
    }
    /* This stores it on the camera again */
    ret = gp_camera_set_config (_gpcamera, widget, _gpcontext);
    if (ret < GP_OK) {
          qDebug(qPrintable(QString("camera_set_config failed: %0\n").arg(ret)));
          return;
    }

    gp_widget_free(widget);
}

QPixmap *QGphotoCamera::getLiveViewImage(){
    CameraFile *file;
    char *data;
    unsigned long datalen;

    QMutexLocker locker(&_lvMutex);
    gp_file_new(&file);

    gp_camera_capture_preview(_gpcamera, file, _gpcontext);


    gp_file_get_data_and_size(file,(const char**) &data, (unsigned long *)&datalen);
    qDebug(qPrintable(QString("Data is %0 bytes").arg(datalen)));

    QPixmap *pix = new QPixmap();
    pix->loadFromData((const unsigned char *)data, datalen);
    gp_file_free(file);

    return pix;

}

int QGphotoCamera::_lookup_widget(CameraWidget*widget, const char *key, CameraWidget **child) {
      int ret;
      ret = gp_widget_get_child_by_name (widget, key, child);
      if (ret < GP_OK)
            ret = gp_widget_get_child_by_label (widget, key, child);
      return ret;
}

void QGphotoCamera::getSettingsInfo(){
        QFile file("gpcameras.xml");
        bool open = file.open(QIODevice::ReadOnly | QIODevice::Text);
        if (!open)
        {
            qDebug("Couldn't open file");
            return;
        }
        else
        {
            qDebug("File opened OK");
        }

        QXmlStreamReader xml(&file);
        qDebug(qPrintable(QString("Encoding: %0").arg(xml.documentEncoding().toString())));
       bool modelFound = false;
       while (!xml.atEnd() && !xml.hasError())
        {
            xml.readNext();
            if (xml.isStartElement())
            {
                QXmlStreamAttributes atts = xml.attributes();
                if(xml.name() == "camera" && atts.value("model") == _model){
                    modelFound = true;
                }
                else if(xml.name() == "camera" && atts.value("model") != _model){
                    modelFound = false;
                }
                if(xml.name() == "setting" && modelFound){
                    QString txtValue = xml.readElementText();
                    qDebug(qPrintable(QString("element name: '%0', text: '%1'").arg(xml.name().toString()).arg(txtValue)));
                    qDebug(qPrintable(QString("id: %0").arg(atts.value("id").toString())));
                    _gpsettings.insert(atts.value("id").toString(), txtValue);
                }
            }
            else if(xml.isEndElement() && xml.name() == "camera" && modelFound){
                break;
            }
            else if (xml.hasError())
            {
                qDebug(qPrintable(QString("XML error: %0").arg(xml.errorString())));
            }
            else if (xml.atEnd())
            {
                qDebug("Reached end, done");
            }

        }
}



