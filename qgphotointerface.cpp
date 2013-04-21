#include "qgphotointerface.h"

QGphotoInterface::QGphotoInterface()

{

    portinfolist = NULL;
    abilities = NULL;
    context = gp_context_new();
}


void QGphotoInterface::initialize(){
    CameraList *list;
    CameraList *xlist;
    Camera     **cams;
          const char *name, *value;
    int ret = gp_list_new(&list);

    if(ret < GP_OK) return;

    if (!portinfolist) {
          /* Load all the port drivers we have... */
          ret = gp_port_info_list_new (&portinfolist);
          if (ret < GP_OK) return ;
          ret = gp_port_info_list_load (portinfolist);
          if (ret < 0) return;
          ret = gp_port_info_list_count (portinfolist);
          if (ret < 0) return;
    }
    /* Load all the camera drivers we have... */
    ret = gp_abilities_list_new (&abilities);
    if (ret < GP_OK) return;
    ret = gp_abilities_list_load (abilities, context);
    if (ret < GP_OK) return;

    /* ... and autodetect the currently attached cameras. */
    ret = gp_list_new(&xlist);
      ret = gp_abilities_list_detect (abilities, portinfolist, xlist, context);
    if (ret < GP_OK) return;

    /* Filter out the "usb:" entry */
      ret = gp_list_count (xlist);
    if (ret < GP_OK) return;
    for (int i=0;i<ret;i++) {


          gp_list_get_name (xlist, i, &name);
          gp_list_get_value (xlist, i, &value);
          if (!strcmp ("usb:",value)) continue;
          gp_list_append (list, name, value);
    }


    gp_list_free(xlist);

    int count = gp_list_count(list);

    cams = (Camera **)calloc (sizeof (Camera*),count);
      for (int i = 0; i < count; i++) {
              gp_list_get_name  (list, i, &name);
              gp_list_get_value (list, i, &value);
            qDebug(qPrintable(QString("Camera: %1 %2").arg(name).arg(value)));

            CameraText  text;
//          ret = sample_open_camera (&cams[i], name, value);
    QGphotoCamera *cam =open_camera(name, value);
        _cameras.append(cam);
          if (ret < GP_OK) fprintf(stderr,"Camera %s on port %s failed to open\n", name, value);
          ret = gp_camera_get_summary (cam->get_gpcamera(), &text, context);
        if (ret < GP_OK) {
              fprintf (stderr, "Failed to get summary.\n");
              continue;
        }
        qDebug(qPrintable(QString("Summary:\n%0\n").arg(text.text)));
      }


    qDebug(qPrintable("Found " +QString("%1").arg(count)+" cameras"));
}
void QGphotoInterface::unload(){}
QList<QCamera *> QGphotoInterface::getcameras(){


    return _cameras;
}

QGphotoCamera *QGphotoInterface::open_camera( const char *model, const char *port)
{
    int         ret, m, p;
    CameraAbilities   a;
    GPPortInfo  pi;
    Camera *camera;
    QGphotoCamera *cam = new QGphotoCamera();
    ret = gp_camera_new (&camera);
    if (ret < GP_OK) return 0;

    /* First lookup the model / driver */
      m = gp_abilities_list_lookup_model (abilities, model);
    if (m < GP_OK) return 0;
      ret = gp_abilities_list_get_abilities (abilities, m, &a);
    if (ret < GP_OK) return 0;
      ret = gp_camera_set_abilities (*(&camera), a);
    if (ret < GP_OK) return 0;

    /* Then associate the camera with the specified port */
      p = gp_port_info_list_lookup_path (portinfolist, port);
      if (ret < GP_OK) return 0;
      switch (p) {
      case GP_ERROR_UNKNOWN_PORT:
              fprintf (stderr, "The port you specified "
                      "('%s') can not be found. Please "
                      "specify one of the ports found by "
                      "'gphoto2 --list-ports' and make "
                      "sure the spelling is correct "
                      "(i.e. with prefix 'serial:' or 'usb:').",
                              port);
              break;
      default:
              break;
      }
      if (ret < GP_OK) return 0;
      ret = gp_port_info_list_get_info (portinfolist, p, &pi);
      if (ret < GP_OK) return 0;
      ret = gp_camera_set_port_info (*(&camera), pi);
      if (ret < GP_OK) return 0;
    _gcams.append(camera);
    cam->set_gpcamera(camera);
    cam->set_identifier(QString(port));
    cam->set_model(QString(model));
    cam->set_gpcontext(context);
    return cam;
}

QString QGphotoInterface::name(){
    return QString("QGphotoCamera");
}
QCamera * QGphotoInterface::selectedCamera(){
    return 0;
}
