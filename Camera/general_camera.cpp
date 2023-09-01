#include "general_camera.h"

General_Camera::General_Camera()
{
    isCapturing=false;
    hasFinished=false;
    hasStarted=false;
}

General_Camera::~General_Camera()
{
    isCapturing=false;
    hasFinished=true;
}

void General_Camera::startCamera()
{
    hasStarted=true;
}

void General_Camera::getOneFrame()
{
    isCapturing=true;
}
