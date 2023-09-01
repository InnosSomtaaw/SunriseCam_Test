#include "image_processing.h"

Image_Processing_Class::Image_Processing_Class()
{
    img_input1,img_output1, img_input2, img_output2,img_output3 = NULL;
    hasInited=false;cam1Refreshed=false;cam2Refreshed = false;isDetecting=false;
    img_filenames = new QList<QFileInfo>();

    save_count = 1,max_save_count =100,onceRunTime=0;

    mainwindowIsNext=false;mainwindowIsStopProcess=false;isSavingImage =false;
    onGPU=false;
}

Image_Processing_Class::~Image_Processing_Class()
{

}

void Image_Processing_Class::iniImgProcessor()
{

}


void Image_Processing_Class::resetPar()
{
    mainwindowIsNext=false;mainwindowIsStopProcess=false;isSavingImage =false;
    save_count=0;
    hasInited=false;cam1Refreshed=false;cam2Refreshed = false;

    img_filenames->clear();

//    hdrProc->hasInitialized=false;
}

void Image_Processing_Class::generalProcess()
{
    usrtimer.start();
    if (img_input1.empty() || !ipcMutex.tryLock())
        return;
    img_output2 = img_input1.clone();
    ipcMutex.unlock();

    vector<Point> cont;
    if(onGPU)
    {
        UMat umIn,umTemp,umOut;
        img_output2.copyTo(umIn);
        pyrDown(umIn,umTemp);

        bilateralFilter(umTemp,umOut,5,40,75);
        adaptiveThreshold(umOut,umTemp,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,5);

//        Mat morpKern=getStructuringElement(MORPH_RECT,Size(11,11));
//        morphologyEx(umTemp,umOut,MORPH_CLOSE,morpKern);
//        morpKern.release();
//        cout<<"Test Morphology time: "<<usrtimer.elapsed()<<" ms with ocl "<<onGPU<<endl;

        pyrUp(umTemp,umOut);
//        umTemp.release();
//        umIn.release();
        umOut.copyTo(img_output1);
    }
    else
    {
        pyrDown(img_output2,img_output3);

        bilateralFilter(img_output3,img_output2,5,40,75);
        adaptiveThreshold(img_output2,img_output3,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,11,5);

//        Mat morpKern=getStructuringElement(MORPH_RECT,Size(11,11));
//        morphologyEx(img_output3,img_output2,MORPH_CLOSE,morpKern);
//        morpKern.release();
//        cout<<"Test Morphology time: "<<usrtimer.elapsed()<<" ms with ocl "<<onGPU<<endl;

        pyrUp(img_output3,img_output1);
    }

//        if (save_count % max_save_count == 0)
//        {
//            imwrite("test1.jpg",img_output1);
//            isSavingImage = true;
//        }
//        else
//            isSavingImage = false;

//        save_count++;
//        if (save_count > max_save_count)
//            save_count = 1;

    emit outputImgProcessedRequest();
    onceRunTime = usrtimer.elapsed();
//    cout<<"Test OCL time: "<<onceRunTime<<" ms with ocl "<<onGPU<<endl;
}

void Image_Processing_Class::startProcessOnce()
{
    switch (workCond) {
    case GeneralProcess:
    default:
        generalProcess();
        break;
    }
}

void Image_Processing_Class::startPicsProcess()
{
    emit mainwindowStatusRequest();
    do
    {
        bool flg = processFilePic();
        if(mainwindowIsStopProcess || !flg)
            break;
        QThread::sleep(10);
        emit mainwindowStatusRequest();
    }while(mainwindowIsNext);
}

void Image_Processing_Class::startMulCamTemp(QImage recvImg, int i)
{
    QImage2Mat(recvImg, img_inputs[i]);
}

void Image_Processing_Class::start1CamProcess(QImage receivedImg)
{
    if(receivedImg.isNull())
        return;
    Mat temp_img(receivedImg.height(),receivedImg.width(),
                CV_8UC4,receivedImg.bits(),
                size_t(receivedImg.bytesPerLine()));
    cvtColor(temp_img,img_input1,COLOR_BGRA2BGR);
//    cvtColor(temp_img,img_input1,CV_BGRA2BGR);
    temp_img.release();
}


void QImage2Mat(QImage img, Mat& imgMat)
{
//    printf("QImage height: %d, width: %d", img.height(), img.width());

//    Mat temp_img(img.height(),img.width(),
//                CV_8UC4,img.bits(),
//                img.bytesPerLine());
//    printf("cvtColor into imgMat");
//    cvtColor(temp_img,imgMat,COLOR_BGRA2BGR);
    Mat temp_img(img.height(), img.width(), CV_8UC3, img.bits(), img.bytesPerLine());
    cvtColor(temp_img, imgMat, COLOR_RGB2BGR);
    temp_img.release();
//    printf("QImage2Mat ended.");
}


void Image_Processing_Class::startMulCamProcess(QImage recvImg, int i)
{
//    if(inputFlags[i])
//    {
//        cout<<i<<" unsync happend."<<endl;
//        return;
//    }
    QImage2Mat(recvImg, img_inputs[i]);
    inputFlags[i] = true;
    for(size_t j = 0; j<4; j++)
    {
        if(!inputFlags[j])
            return;
    }
//    testMulCamAIProcess();
//    testMulStereoMatcher();
}


void Image_Processing_Class::testMulStereoMatcher()
{
////    printf(" testMulStereoMatcher start");
//    if(!ipcMutex.tryLock())
//    {
//        printf("no lock!");
//        return;
//    }
//    if(img_inputs[2].empty() | img_inputs[3].empty())
//    {
//        printf("image inputs are empty.");
//        return;
//    }
//    vector<Point> points;
//    points.push_back(Point(276, 288));
//    points.push_back(Point(278, 288));
//    vector<Point3f> pts3;
//    stereoMatcher.calcXYZ(img_inputs[2], img_inputs[3], points, pts3);
//    pt1 = pts3[0];
//    pt2 = pts3[1];
//    printf("%f, %f, %f\n", pt1.x, pt1.y, pt1.z);
//    for(int k = 0; k<4; k++)
//    {
////        resize(img_inputs[k], img_inputs[k], Size(this_aue->SrcImage.width, this_aue->SrcImage.height));
//        img_outputs[k] = img_inputs[k].clone();
//    }
//    for(int k = 0; k<4; k++)
//        inputFlags[k] = false;

//    ipcMutex.unlock();
//    emit outputMulImgAIRequest();
}

void Image_Processing_Class::changeProcPara(QString qstr, int wc)
{
//    switch (wc) {
//    case WorkConditionsEnum::HDRfrom1Img:
//    case WorkConditionsEnum::HDRfrom1ImgGPU:
//        hdrProc->idConst=qstr.toFloat();
//        break;
//    default:
//        break;
//    }

}

bool Image_Processing_Class::processFilePic()
{
    if (img_filenames->count() > 0)
    {
        img_input1 = imread(img_filenames->at(0).filePath().toLocal8Bit().toStdString());

        if(img_input1.empty())
            return false;

        startProcessOnce();

        img_filenames->removeAt(0);
        return true;
    }
    else
    {
        return false;
    }
}
