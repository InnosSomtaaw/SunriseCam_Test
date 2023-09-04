QT       += core gui network xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Camera/general_camera.cpp \
    Camera/videoplayer.cpp \
    Camera/MvCamera.cpp \
    ImageProcess/image_processing.cpp \
    ImageProcess/imghdr.cpp \
    main.cpp \
    mainwindow.cpp \
    pictureview.cpp

HEADERS += \
    Camera/general_camera.h \
    Camera/videoplayer.h \
    Camera/MvCamera.h \
    ImageProcess/image_processing.h \
    ImageProcess/imghdr.h \
    mainwindow.h \
    pictureview.h

FORMS += \
    mainwindow.ui

TARGET = SunriseCam_Test
TEMPLATE = app
target.path = /home/sunrise/CrossCompiled/$${TARGET}
INSTALLS += target

QMAKE_LFLAGS += -no-pie

unix:!macx: LIBS += \
-L/home/sunrise/opencv-build/install/lib/ -lopencv_world \
-L/home/sunrise/ffmpegInstall/lib/ -lavcodec \
-L/home/sunrise/ffmpegInstall/lib/ -lavformat \
-L/home/sunrise/ffmpegInstall/lib/ -lavutil \
-L/home/sunrise/ffmpegInstall/lib/ -lswresample \
-L/home/sunrise/ffmpegInstall/lib/ -lswscale \
-L/opt/MVS/lib/aarch64 -lMvCameraControl \
-L/opt/MVS/bin

INCLUDEPATH += \
/home/sunrise/ffmpegInstall/include \
/home/sunrise/opencv-build/install/include/opencv4 \
/opt/MVS/include
DEPENDPATH += \
/home/sunrise/ffmpegInstall/include \
/home/sunrise/opencv-build/install/include/opencv4 \
/opt/MVS/include

