#-------------------------------------------------
#
# Project created by QtCreator 2013-04-17T21:46:13
#
#-------------------------------------------------

QT  += core widgets gui

TARGET = QGphotoCamera
TEMPLATE = lib
DESTDIR = ./cameras
DEFINES += QGPHOTOCAMERA_LIBRARY NODEPS __MINGW__ USE_CIMG

SOURCES += qgphotocamera.cpp \
    qgphotointerface.cpp

HEADERS += qgphotocamera.h\
        qgphotocamera_global.h \
    qgphotointerface.h
INCLUDEPATH += ../electrobee/QCameraInterface ../DcRawQT /Users/bowesjs/0.2/5.0.2/clang_64/include /opt/local/include

LIBS += -L"../Debug" -L"/opt/local/lib" -lDcRawQT -lQCameraInterface -lgphoto2 -lgphoto2_port
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    gpcameras.xml
