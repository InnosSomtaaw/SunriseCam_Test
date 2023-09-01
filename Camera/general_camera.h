#ifndef GENERAL_CAMERA_H
#define GENERAL_CAMERA_H

#include <iostream>
#include <QThread>
#include <QString>
#include <QImage>
#include <QMutex>

QT_BEGIN_NAMESPACE
using namespace std;
QT_END_NAMESPACE

class General_Camera : public QThread
{
    Q_OBJECT

public:
    General_Camera();
    ~General_Camera();

    int isCapturing,hasFinished,hasStarted;
    QString camURL;
    QMutex camMutex;

    virtual void startCamera();
    virtual void getOneFrame();

signals:
    void sigGetOneFrame(QImage);

};

#endif // GENERAL_CAMERA_H
