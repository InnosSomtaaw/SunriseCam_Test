#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //显示窗体初始化
    initImageBoxes();
    //初始化程序工况
    initWorkCondition();
    //文本输出初始化
    initTextBrowsers();
    //初始化参数设置
    initBaseSettings();
    //图像处理类初始化
    initImageProcess();
    //初始化网络相机设置
    initCamSettings();
    //数字相机类初始化
    initVideoPlayers();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initCamSettings()
{
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h265/ch1/sub/av_stream");
    urls.push_back("rtsp://admin:Lead123456@192.168.137.98:554/h265/ch1/main/av_stream");

    // ch:枚举子网内所有设备 | en:Enumerate all devices within subnet
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    // ch:将值加入到信息列表框中并显示出来 | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        QString strMsg;
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }

        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName) != 0)
            {
                strMsg.sprintf("[%d]GigE:   %s  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName, nIp1, nIp2, nIp3, nIp4);
            }
            else
            {
                strMsg.sprintf("[%d]GigE:   %s %s (%s)  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
                               pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber, nIp1, nIp2, nIp3, nIp4);
            }
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
            {
                strMsg.sprintf("[%d]UsbV3:  %s", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
            }
            else
            {
                strMsg.sprintf("[%d]UsbV3:  %s %s (%s)", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
                               pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
        }
        else
        {
            ShowErrorMsg("Unknown device enumerated", 0);
        }
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg("No device", 0);
        return;
    }
}

void MainWindow::initImageBoxes()
{
    ui->imagebox1->setScene(&scene1);
    scene1.addItem(&pixmapShow1);
    connect(this->ui->imagebox1,&PictureView::outputImgProperty_request,
            this,&MainWindow::on_imagebox1_OpenImage);
}

void MainWindow::initImageProcess()
{
    imgProcHDR=new IMG_HDR;
    imgProcHDR->workCond=workCond;
    switch (workCond) {
    case HDRfrom2Img:
    case HDRfrom2ImgGPU:
    case HDRfrom1Img:
    {
        connect(this,&MainWindow::startCam1Request,
                imgProcHDR,&IMG_HDR::startProcessOnce);
        connect(imgProcHDR,&IMG_HDR::outputImgProcessedRequest,
                this,&MainWindow::on_imagebox1_refresh);
        imgProcessor1=imgProcHDR;
        break;
    }
    case InferenceRKNN:
    case GeneralProcess:
    default:
    {
        imgProcessor1=new Image_Processing_Class;
        imgProcessor1->workCond=workCond;
        connect(this,&MainWindow::startCam1Request,
                imgProcessor1,&Image_Processing_Class::startProcessOnce);
        connect(imgProcessor1,&Image_Processing_Class::outputImgProcessedRequest,
                this,&MainWindow::on_imagebox1_refresh);
        break;
    }
    }
    imgProThread1 = new QThread();
    imgProcessor1->moveToThread(imgProThread1);
    imgProThread1->start();

    imgProcessor1->onGPU=true;
    ocl::setUseOpenCL(imgProcessor1->onGPU);
}

void MainWindow::initVideoPlayers()
{
//    //数字相机类初始化（HIKVISION读取）
//    cam1=new CMvCamera;
//    if(m_stDevList.nDeviceNum<2)
//        return;
//    cam1->m_stDevList=m_stDevList;
//    cam1->startCamera();
//    connect(cam1,&CMvCamera::sigGetOneFrame,
//            this,&MainWindow::slotGetOneFrame1);

//    //数字相机类初始化（ffmpeg读取）
    cam1=new VideoPlayer;
    cam1->camURL=urls[0];
    connect(cam1,&VideoPlayer::sigGetOneFrame,
            this,&MainWindow::slotGetOneFrame1);
}

void MainWindow::initBaseSettings()
{
    isDetecting=false;
    detecOnly1st=false;

//    for(FileNodeIterator it = fnode.begin(); it!=fnode.end(); it++)
//    {
//        Point2f p;
//        *it>>p;
////        cout<<p.x<<", "<<p.y<<endl;
//        basePoints.push_back(p);
//    }
//    return;
}

void MainWindow::initTextBrowsers()
{
    ui->textBrowser1->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    strOutput1="place holder 1...\n";
    ui->textBrowser1->setText(strOutput1);
}

void MainWindow::initWorkCondition()
{
    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    workCond=WorkConditionsEnum(iniRW->value("WorkCondition/WorkCondition").toInt());
    proj_path=iniRW->value("InferenceRKNN/ModelPath").toString();
    ui->condComboBox->setCurrentIndex(workCond);
}

void MainWindow::on_buttonOpenAIProject_clicked()
{
    proj_path = QFileDialog::getOpenFileName(this,tr("Open RKNN Model: "),"./model/",
                                             tr("Model File(*.rknn)"));
    if (proj_path.isEmpty())
        return;
//    proj_path = proj_path.replace("/","\");
}

void MainWindow::on_imagebox1_OpenImage()
{
    QString fileName = ui->imagebox1->img_Name_input;
    if (fileName.isEmpty())
        return;

//    Mat srcImage = imread(fileName.toLatin1().data());//读取图片数据
    Mat srcImage = imread(fileName.toLocal8Bit().toStdString());//读取图片数据

    img_input1 = srcImage.clone();
    imgProcessor1->img_input1=img_input1;

    cvtColor(srcImage, srcImage, COLOR_BGR2RGB);//图像格式转换
    QImage disImage = QImage(srcImage.data,srcImage.cols,srcImage.rows,
                             QImage::Format_RGB888);

    //窗体1显示
    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));

    ui->imagebox1->Adapte();
    srcImage.release();
}

void MainWindow::on_buttonProcess_clicked()
{
    if(imgProcessor1->img_filenames->size()==0
        && !cam1->isCapturing)
    {
        isDetecting=true;
        imgProcessor1->isDetecting=isDetecting;
        imgProcessor1->startProcessOnce();
        return;
    }

    if(ui->buttonProcess->text()=="Process" && isDetecting==false)
    {
        isDetecting=true;
        imgProcessor1->isDetecting=isDetecting;
        ui->buttonProcess->setText("StopProcess");
        ui->condComboBox->setEnabled(false);

        imgProcessor1->ipcMutex.unlock();

//        if (ui->buttonOpenImageList->text() == "Next")
//            if(!this->m_imgprocsThread->isInterruptionRequested())
//                emit startPicsProcessRequest();

    }
    else if (ui->buttonProcess->text() == "StopProcess" && isDetecting == true)
    {
        isDetecting=false;
        imgProcessor1->isDetecting=isDetecting;
        ui->buttonProcess->setText("Process");
        ui->condComboBox->setEnabled(true);

        imgProcessor1->ipcMutex.tryLock();
    }
}

void MainWindow::on_buttonReset_clicked()
{
//    ui->buttonOpenImageList->setText("OpenImageList");
//    ui->buttonOpenVideo->setText("OpenVideo");
    cam1->isCapturing=false;
    printf("reset clicked!");

    ui->buttonOpenImgList->setText("OpenImgList");
    isDetecting = false;
    ui->buttonProcess->setText("Process");
    ui->buttonStartCapture->setText("StartCapture");
    imgProcessor1->resetPar();
    ui->buttonOpenAIProject->setEnabled(true);
}

void MainWindow::on_textSavingCount_textChanged()
{
    QString qstr = ui->textSavingCount->toPlainText();
    switch (workCond) {
    case HDRfrom1Img:
    case HDRfrom1ImgGPU:
        emit changeProcParasRequest(qstr,workCond);
        break;
    default:
        imgProcessor1->max_save_count = qstr.toInt();
        break;
    }
}

void MainWindow::on_buttonStartCapture_clicked()
{  
    if(ui->buttonStartCapture->text()=="StartCapture")
    {
        cam1->isCapturing=true;
        ui->buttonStartCapture->setText("StopCapture");

        if(!cam1->hasStarted)
            cam1->startCamera();
        else
            cam1->camMutex.unlock();

    }
    else if(ui->buttonStartCapture->text()=="StopCapture")
    {
        cam1->isCapturing=false;

        ui->buttonStartCapture->setText("StartCapture");

        cam1->camMutex.tryLock();

//        if((!imgProcess_main->img_inputs[0].empty())&(!imgProcess_main->img_inputs[2].empty()))
//        {
//            ui->editCheckBox->setEnabled(true);
//        }
    }
}

void MainWindow::on_imagebox1_refresh()
{
    imgProcessor1->img_output1.copyTo(img_output1);
    QImage disImage;
//    窗体1显示
    if(img_output1.channels()==1)
        disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                          img_output1.cols*img_output1.channels(),QImage::Format_Grayscale8);
    else
        disImage = QImage(img_output1.data,img_output1.cols,img_output1.rows,
                          img_output1.cols*img_output1.channels(),QImage::Format_RGB888);
    pixmapShow1.setPixmap(QPixmap::fromImage(disImage));
    int gap2recv=time1.elapsed();
    time1.start();
    //界面状态显示
    QString tempStr;
    tempStr="1st camera refresh time(processed): "+QString::number(gap2recv)+
            "ms. In which process consumed: "+QString::number(imgProcessor1->onceRunTime)+
            "ms.\n";
    strOutput1+=tempStr;
    double fps=1000/double(gap2recv);
    tempStr="1st camera FPS(processed): "+QString::number(fps,'f',1)+".\n";
    strOutput1+=tempStr;
    ui->textBrowser1->setText(strOutput1);
    strOutput1.clear();
    tempStr.clear();
}

void MainWindow::slotGetOneFrame1(QImage img)
{
    if(cam1->isCapturing && !isDetecting)
    {
        //窗体1显示
        pixmapShow1.setPixmap(QPixmap::fromImage(img));
//        ui->imagebox1->Adapte();
        int gap2recv=time1.elapsed();
        time1.start();
        QString tempStr;
        tempStr="\n1st camera refresh time: "+QString::number(gap2recv)+" ms.\n";
        strOutput1+=tempStr;
        double fps=1000/double(gap2recv);
        tempStr="1st camera FPS: "+QString::number(fps,'f',1)+".\n";
        strOutput1+=tempStr;
        ui->textBrowser1->setText(strOutput1);
        strOutput1.clear();
        tempStr.clear();
    }

    if(cam1->isCapturing && isDetecting)
    {
        img_input1=Mat(img.height(), img.width(), CV_8UC1,
                        img.bits(), img.bytesPerLine());
        if(imgProcessor1->ipcMutex.tryLock())
        {
            img_input1.copyTo(imgProcessor1->img_input1);
            imgProcessor1->ipcMutex.unlock();
            emit startCam1Request();
        }
    }
}

void MainWindow::on_buttonOpenImgList_clicked()
{
    if(ui->buttonOpenImgList->text()=="OpenImgList")
    {
        QString filename = QFileDialog::getExistingDirectory();
        QDir *dir=new QDir(filename);
        QStringList filter;
        filter<<"*.png"<<"*.jpg"<<"*.jpeg"<<"*.bmp";

        imgProcessor1->img_filenames =new QList<QFileInfo>(dir->entryInfoList(filter));

        imgProcessor1->processFilePic();
        if(imgProcessor1->img_filenames->count()>0)
            ui->buttonOpenImgList->setText("Next");
    }
    else if(ui->buttonOpenImgList->text()=="Next")
    {
        if (!imgProcessor1->processFilePic())
            ui->buttonOpenImgList->setText("OpenImgList");
    }
}

void MainWindow::on_condComboBox_activated(int index)
{
    ui->buttonReset->click();
    workCond = (WorkConditionsEnum)index;
    switch (workCond) {
    case HDRfrom2Img:
    case HDRfrom2ImgGPU:
    case HDRfrom1Img:
    {
        connect(this,&MainWindow::startCam1Request,
                imgProcHDR,&IMG_HDR::startProcessOnce);
        connect(imgProcHDR,&IMG_HDR::outputImgProcessedRequest,
                this,&MainWindow::on_imagebox1_refresh);
        imgProcessor1=imgProcHDR;
        break;
    }
    case InferenceRKNN:
    case GeneralProcess:
    default:
    {
        if(imgProcessor1!=nullptr)
            delete imgProcessor1;
        imgProcessor1=new Image_Processing_Class;
        connect(this,&MainWindow::startCam1Request,
                imgProcessor1,&Image_Processing_Class::startProcessOnce);
        connect(imgProcessor1,&Image_Processing_Class::outputImgProcessedRequest,
                this,&MainWindow::on_imagebox1_refresh);
        break;
    }
    }
    imgProcessor1->workCond=workCond;

    iniRW = new QSettings("LastSettings.ini",QSettings::IniFormat);
    iniRW->setValue("WorkCondition/WorkCondition",index);
    iniRW->setValue("InferenceRKNN/ModelPath",proj_path);
}
