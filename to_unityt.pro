#-------------------------------------------------
#
# Project created by QtCreator 2019-06-04T14:43:46
#
#-------------------------------------------------

QT       += opengl

QT       -= gui

#TARGET = D:/i2cat_vrtogether/to_unityt/debug/to_unityt
TARGET = to_unityt

TEMPLATE = lib

LIBS += -L"C:/Program Files (x86)/Windows Kits/10/lib/10.0.17134.0/um/x64/" \
-lopengl32 \
-LD:/i2cat_vrtogether/to_unityt/ \
-lsdl/lib/x64/SDL \
-lcwipc_codec/lib/Debug/cwipc_codec \
-lcwipc_codec/lib/Debug/cloud_codec_v2 \
-lcwipc_codec/lib/Debug/pcl_jpeg_io \
-lcwipc_util/lib/Debug/cwipc_util

DEFINES += TO_UNITYT_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        to_unityt.cpp \
    CapturSUB.cpp \
    CapturSUBThread.cpp \
    GLMessages.cpp \
    GLVertexBuffers.cpp \
    TimerHQ.cpp

HEADERS += \
        to_unityt.h \
        to_unityt_global.h \ 
    CapturSUB.h \
    CapturSUBThread.h \
    GLMessages.h \
    GLVertexBuffers.h \
    qvector3duchar.h \
    qvector4duchar.h \
    signals_unity_bridge.h \
    TimerHQ.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
