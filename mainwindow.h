#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtWidgets>
#include <QtXml/QDomDocument>
#include <QTextStream>

#include "Camera/MvCamera.h"
#include "Camera/videoplayer.h"
#include "ImageProcess/image_processing.h"
#include "ImageProcess/imghdr.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow;}
using namespace cv;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isDetecting,detecOnly1st;
    QElapsedTimer time1;

private:
    Ui::MainWindow *ui;
    Mat img_input1,img_output1;

    //网络相机参数
    vector<QString> urls;
    MV_CC_DEVICE_INFO_LIST  m_stDevList; 
    //AI工程相关文件路径
    QString proj_path;

    vector<Point2i> basePoints;
    WorkConditionsEnum workCond;
    QSettings *iniRW;
//    //数字相机类（HIKVISION相机）
//    CMvCamera *cam1;
    //数字相机类（ffmpeg读取）
    VideoPlayer *cam1;
    //图像处理类
    Image_Processing_Class *imgProcessor1;
    IMG_HDR *imgProcHDR;
    //图像处理线程
    QThread *imgProThread1;

    QGraphicsScene scene1;
    QGraphicsPixmapItem pixmapShow1;
    QString strOutput1;
    //显示窗体初始化
    void initImageBoxes();
    //初始化程序工况
    void initWorkCondition();
    //文本输出初始化
    void initTextBrowsers();
    //初始化参数设置
    void initBaseSettings();
    //图像处理类初始化
    void initImageProcess();
    //初始化网络相机设置
    void initCamSettings();
    //数字相机类初始化
    void initVideoPlayers();

private slots:
    //窗体1打开图片
    void on_imagebox1_OpenImage();
    //选择AI工程和节点按钮槽
    void on_buttonOpenAIProject_clicked();
    //重置按钮槽
     void on_buttonReset_clicked();
    //实时处理按钮槽
    void on_buttonProcess_clicked();
    //图片1刷新槽
    void on_imagebox1_refresh();
    //刷新图片保存参数文本框槽
    void on_textSavingCount_textChanged();
    //开始连续采集按钮槽
    void on_buttonStartCapture_clicked();
    //视频流1获取槽
    void slotGetOneFrame1(QImage img);
    //打开图片序列按钮槽
    void on_buttonOpenImgList_clicked();
    //切换处理算法槽
    void on_condComboBox_activated(int index);

signals:
    //开始单次处理信号
    void startCam1Request();
    //开始图片组处理信号
    void startPicsProcessRequest();
    //处理参数改变信号
    void changeProcParasRequest(QString,int);
};
#endif // MAINWINDOW_H
