#ifndef __LEAD_SHUFFLENET_H__
#define __LEAD_SHUFFLENET_H__

#include "ImageProcess/image_processing.h"
#include "ImageProcess/bpu_resize.h"

#include <dnn/hb_dnn.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>

class Lead_Horizon:public Image_Processing_Class
{
    Q_OBJECT
public:
    Lead_Horizon() = default;;
    ~Lead_Horizon();

    const char* model_name = NULL;
    int rawW,rawH,outputW,outputH,classNums;
    bool input_is_bayer=false;

    int32_t Init(const char* modelFile_path, int img_w, int img_h);
    cv::Mat  Forward(cv::Mat &img);

public slots:
    void iniImgProcessor() override;
    void startProcessOnce() override;

private:
    hbDNNHandle_t dnn_handle;
    hbPackedDNNHandle_t packed_dnn_handle;
    int input_count = 0;
    int output_count = 0;
    std::vector<hbDNNTensor> input_tensors;
    std::vector<hbDNNTensor> output_tensors;
    std::vector<hbDNNTensor> res;
    int32_t prepare_tensor();
    int32_t read_image_2_tensor(float *raw_data);
    cv::Mat Post_Process(hbDNNTensor *output, cv::Mat res);
    int input_h = 0;
    int input_w = 0;
    int input_c = 0;
    int32_t pic_size = 0;
    int num_classes_ = 0;
    cv::Mat mat;
    BpuResize* resizer;
};
#endif
