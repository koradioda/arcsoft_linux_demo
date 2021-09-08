QT       += core gui sql
QT       += multimedia
QT       += multimediawidgets
QT       += concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#LIBS += \
#    /usr/local/lib/libarcsoft_face.so \
#    /usr/local/lib/libarcsoft_face_engine.so
LIBS += \
      ./lib/libarcsoft_face.so \
      ./lib/libarcsoft_face_engine.so

SOURCES += \
    arcfacedemo.cpp \
    facedetect.cpp \
    facesdatabase.cpp \
    livenessrecognize.cpp \
    main.cpp \
    mutilface.cpp \
    threadcam.cpp \
    videoframecapture.cpp \
    videoft.cpp \
    viewlabel.cpp

HEADERS += \
    arcfacedemo.h \
    facedetect.h \
    facesdatabase.h \
    inc/amcomdef.h \
    inc/arcsoft_face_sdk.h \
    inc/asvloffscreen.h \
    inc/merror.h \
    livenessrecognize.h \
    mutilface.h \
    threadcam.h \
    videoframecapture.h \
    videoft.h \
    viewlabel.h

FORMS += \
 arcfacedemo.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    lib/libarcsoft_face.so \
    lib/libarcsoft_face_engine.so
