#ifndef __BPU_RESIZE__
#define __BPU_RESIZE__

#include <iostream>
#include <string>
#include <initializer_list>
#include "dnn/hb_dnn.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
using namespace std;

#define ALIGNED_2E(w, alignment) ((w) + (alignment - 1)) & (~(alignment - 1))
#define ALIGN_16(w) ALIGNED_2E(w, 16)

int32_t prepare_input_tensor(
                           int image_height,
                           int image_width,
                           hbDNNTensor *tensor,int& stride,int& input_image_length);
int32_t prepare_output_tensor(int image_height,
                           int image_width,
                           hbDNNTensor *tensor,int& output_image_length);
int32_t free_tensor(hbDNNTensor *tensor);


enum class imageType : int {
    BGR = 0,
    RGB = 1,
    YUV420 = 2,
    NV12 = 3,
};

class BpuResize{
// 固定尺寸 支持 brg rgb yuv420 nv12

public:
    BpuResize(const BpuResize& other) = delete; 
    BpuResize& operator = (const BpuResize& other) = delete;
    explicit BpuResize(const int input_w, const int input_h, const int output_w, const int output_h, const imageType imgType);
    ~BpuResize();
    void copy_image_2_input_tensor(uint8_t *image_data,hbDNNTensor *tensor);
    float *Resize(cv::Mat ori_img, const std::initializer_list<int> crop={});
    void Resize(cv::Mat ori_img, cv::Mat &dst_img);
    int32_t prepare_input_tensor();
    int32_t prepare_output_tensor();

private:
    int inputW;
    int inputH;
    int outputW;
    int outputH;
    int Dimensions;
    int shape_0, shape_1, shape_2, shape_3;
    imageType imgType;
    hbDNNDataType dataType;
    hbDNNTensorLayout layout;

    hbDNNTensor input_tensor;
    hbDNNTensor output_tensor;
    hbDNNResizeCtrlParam ctrl;
    hbDNNTaskHandle_t task_handle;
    hbDNNRoi roi;
    bool useCrop = false;
    int input_image_length;
    int output_image_length;

    int stride;
};
#endif

