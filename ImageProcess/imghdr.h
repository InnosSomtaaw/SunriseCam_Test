#ifndef IMGHDR_H
#define IMGHDR_H

#include "ImageProcess/image_processing.h"

class IMG_HDR : public Image_Processing_Class
{
    Q_OBJECT
public:
    IMG_HDR();
    ~IMG_HDR();

//signals:
//    //刷新主窗体显示后图片信号
//    void outputImgProcessedRequest();
//    //刷新主窗体多画面显示信号AI测试用
//    void outputMulImgAIRequest();
//    //请求主窗体按钮状态信号
//    void mainwindowStatusRequest();

public:
    Mat coffMat,coffMatC2,mat4derivate_r3c1,mat4derivate_r1c3;
    float idConst;

    void create_coff_mat();

    void hdr2Imgs();
    void hdr1Img();

private:
    //双灰度图片HDR合成
    Mat hdr2GrayImgs(Mat img_src_bright, Mat img_src_dark);
    //单灰度图片HDR合成
    Mat hdr1GrayImgs(Mat img);

    //获取最大梯度图像
    void get_max_derivation(Mat bright_img, Mat dark_img,
                            Mat &max_derivativeX, Mat &max_derivativeY);
    //增强梯度图像
    void enhanceDerivation(Mat img,Mat &max_derivativeX, Mat &max_derivativeY);
    //傅里叶变换数值求解泊松方程
    Mat second_method_solving_Possion(Mat derivativeX, Mat derivativeY);

};
#endif // IMGHDR_H
