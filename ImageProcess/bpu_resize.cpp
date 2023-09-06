#include "bpu_resize.h"


BpuResize::BpuResize(const int input_w,const int input_h,const int output_w,const int output_h,const imageType imgType){
    this->imgType = imgType;
    inputW = input_w;   
    outputW = output_w; 
    if(imgType == imageType::YUV420){
        inputH = input_h * 1.5;
        outputH = output_h * 1.5;
        dataType = HB_DNN_IMG_TYPE_Y;
        layout = HB_DNN_LAYOUT_NCHW;
        Dimensions = 4;
        shape_0 = 1;
        shape_1 = 1;
        shape_2 = inputH;
        shape_3 = inputW;

    }
    else if(imgType == imageType::BGR || imgType == imageType::RGB){
        dataType = this->imgType==imageType::BGR ? HB_DNN_IMG_TYPE_BGR:HB_DNN_IMG_TYPE_RGB;
        inputH = input_h;
        outputH = output_h;
        layout = HB_DNN_LAYOUT_NHWC;
        Dimensions = 4;
        shape_0 = 1;
        shape_1 = inputH;
        shape_2 = inputW;
        shape_3 = 3;

    }
    prepare_input_tensor();
    prepare_output_tensor();
}
BpuResize::~BpuResize(){
    hbSysFreeMem(&input_tensor.sysMem[0]);
    hbSysFreeMem(&output_tensor.sysMem[0]);
}

int32_t BpuResize::prepare_input_tensor() {
        
    auto &properties = this->input_tensor.properties;
    auto &aligned_shape = properties.alignedShape;
    auto &valid_shape = properties.validShape;

    if(this->imgType == imageType::YUV420){
        properties.tensorType = HB_DNN_IMG_TYPE_Y;
        properties.tensorLayout = HB_DNN_LAYOUT_NCHW;
        valid_shape.numDimensions = Dimensions;
        valid_shape.dimensionSize[0] = shape_0;
        valid_shape.dimensionSize[1] = shape_1;
        valid_shape.dimensionSize[2] = shape_2;
        valid_shape.dimensionSize[3] = shape_3;
        stride = ALIGN_16(inputW);
        aligned_shape.numDimensions = Dimensions;
        aligned_shape.dimensionSize[0] = shape_0;
        aligned_shape.dimensionSize[1] = shape_1;
        aligned_shape.dimensionSize[2] = shape_2;
        aligned_shape.dimensionSize[3] = stride;
    }
    else if (this->imgType == imageType::BGR||this->imgType == imageType::RGB){
            properties.tensorType = this->imgType==imageType::BGR ?HB_DNN_IMG_TYPE_BGR:HB_DNN_IMG_TYPE_RGB;
            properties.tensorLayout = HB_DNN_LAYOUT_NHWC;
            valid_shape.numDimensions = Dimensions;
            valid_shape.dimensionSize[0] = shape_0;
            valid_shape.dimensionSize[1] = shape_1;
            valid_shape.dimensionSize[2] = shape_2;
            valid_shape.dimensionSize[3] = shape_3;
    }
    aligned_shape = valid_shape;
    input_image_length = aligned_shape.dimensionSize[1]*
                        aligned_shape.dimensionSize[2]*
                        aligned_shape.dimensionSize[3];
    hbSysAllocCachedMem(&this->input_tensor.sysMem[0], input_image_length);
    return 0;
}


int32_t BpuResize::prepare_output_tensor()
{
    auto &properties = this->output_tensor.properties;
    auto &valid_shape = properties.validShape;
    auto &aligned_shape = properties.alignedShape;
    if (this->imgType == imageType::YUV420)
    {
        properties.tensorType = HB_DNN_IMG_TYPE_Y;
        properties.tensorLayout = HB_DNN_LAYOUT_NCHW;
        valid_shape.numDimensions = Dimensions;
        valid_shape.dimensionSize[0] = shape_0;
        valid_shape.dimensionSize[1] = shape_1;
        valid_shape.dimensionSize[2] = outputH;
        valid_shape.dimensionSize[3] = outputW;
    }
    else if (this->imgType == imageType::BGR || this->imgType == imageType::RGB)
    {
        properties.tensorType = this->imgType == imageType::BGR ? HB_DNN_IMG_TYPE_BGR : HB_DNN_IMG_TYPE_RGB;
        properties.tensorLayout = HB_DNN_LAYOUT_NHWC;
        valid_shape.numDimensions = Dimensions;
        valid_shape.dimensionSize[0] = shape_0;
        valid_shape.dimensionSize[1] = outputH;
        valid_shape.dimensionSize[2] = outputW;
        valid_shape.dimensionSize[3] = shape_3;
    }
    aligned_shape = valid_shape;
    output_image_length = aligned_shape.dimensionSize[1] *
                          aligned_shape.dimensionSize[2] *
                          aligned_shape.dimensionSize[3];
    hbSysAllocCachedMem(&this->output_tensor.sysMem[0], output_image_length);
    return 0;
}
float* BpuResize::Resize(cv::Mat ori_img,const std::initializer_list<int> crop){
    if(crop.size()>0){
        assert(crop.size()==4);
        vector<int>tmp = crop;
        useCrop = true;
        roi= {tmp[0],tmp[1],tmp[2],tmp[3]};
    }
    HB_DNN_INITIALIZE_RESIZE_CTRL_PARAM(&ctrl);
    copy_image_2_input_tensor(ori_img.data,&input_tensor);
    if (!useCrop)
    {
        hbDNNResize(&task_handle, &output_tensor, &input_tensor, nullptr, &ctrl);
    }
    else
    {
        hbDNNResize(&task_handle, &output_tensor, &input_tensor, &roi, &ctrl);
    }

    hbDNNWaitTaskDone(task_handle, 0);
    hbDNNReleaseTask(task_handle);
    float* res  = reinterpret_cast<float *>(output_tensor.sysMem[0].virAddr);
    return res;
}

void BpuResize::Resize(cv::Mat ori_img, cv::Mat &dst_img)
{
    HB_DNN_INITIALIZE_RESIZE_CTRL_PARAM(&ctrl);
    copy_image_2_input_tensor(ori_img.data,&input_tensor);
    hbDNNResize(&task_handle, &output_tensor, &input_tensor, nullptr, &ctrl);
    hbDNNWaitTaskDone(task_handle, 0);
    hbDNNReleaseTask(task_handle);

    dst_img=cv::Mat(outputH, outputW, ori_img.type());
    memcpy(dst_img.data,output_tensor.sysMem[0].virAddr,
            dst_img.rows*dst_img.cols*dst_img.channels());

//    float* res  = reinterpret_cast<float *>(output_tensor.sysMem[0].virAddr);
//    dst_img=cv::Mat(outputH, outputW, CV_32FC(ori_img.channels()));
//    memcpy(dst_img.data,res,output_tensor.sysMem[0].memSize);
//    dst_img.convertTo(dst_img, CV_8UC(dst_img.channels()));
}

void BpuResize::copy_image_2_input_tensor(uint8_t *image_data,hbDNNTensor *tensor){
    uint8_t *data0 = reinterpret_cast<uint8_t *>(tensor->sysMem[0].virAddr);
    memcpy(data0, image_data, input_image_length);
    hbSysFlushMem(&(tensor->sysMem[0]), HB_SYS_MEM_CACHE_CLEAN);
}
