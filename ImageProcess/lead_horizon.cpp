#include "lead_horizon.h"
#include "time.h"

Lead_Horizon::~Lead_Horizon(){
    for (int i = 0; i < input_count; i++) {
        hbSysFreeMem(&(input_tensors[i].sysMem[0]));
    }
    for (int i = 0; i < output_count; i++) {
        hbSysFreeMem(&(output_tensors[i].sysMem[0]));
    }
    hbDNNRelease(packed_dnn_handle);
}

int32_t Lead_Horizon::Init(const char* modelFile_path, int img_w, int img_h){
	if(hbDNNInitializeFromFiles(&packed_dnn_handle, &modelFile_path, 1))
		return 1;
	const char **model_name_list;
	int model_count = 0;
	if(hbDNNGetModelNameList(&model_name_list, &model_count, packed_dnn_handle))
		return 1;
	if(hbDNNGetModelHandle(&dnn_handle, packed_dnn_handle, model_name_list[0]))
		return 1;
	if(hbDNNGetInputCount(&input_count, dnn_handle)) return 1;
	//assert(input_count == 1 && "only input_count 1 is avalid!");
	if(input_count != 1){
		printf("only input_count 1 is avalid!\n");
		return 1;
	}
	if(hbDNNGetOutputCount(&output_count, dnn_handle)) return 1;
	input_tensors.resize(input_count);
	output_tensors.resize(output_count);
	if(prepare_tensor()) return 1;
	pic_size = input_h * input_w * input_c;
	std::cout << input_h << " , " << input_w << std::endl;
	resizer = new BpuResize(img_w, img_h, input_w, input_h,imageType::BGR);
	mat = cv::Mat::zeros(input_h , input_w, CV_8UC3);
	return 0;
}

cv::Mat Lead_Horizon::Forward(cv::Mat &img){

	cv::Mat res;
	float* data = resizer->Resize(img);
	read_image_2_tensor(data);
	hbDNNTaskHandle_t task_handle = nullptr;
	hbDNNTensor *output = output_tensors.data();
	for (int i = 0; i < input_count; i++) {
		hbSysFlushMem(&input_tensors[i].sysMem[0], HB_SYS_MEM_CACHE_CLEAN);
	}
	hbDNNInferCtrlParam infer_ctrl_param;
	HB_DNN_INITIALIZE_INFER_CTRL_PARAM(&infer_ctrl_param);
	if(hbDNNInfer(&task_handle,&output,input_tensors.data(),dnn_handle,&infer_ctrl_param))
		return res;
	if(hbDNNWaitTaskDone(task_handle, 0))
		return res;
	for (int i = 0; i < output_count; i++) {
		hbSysFlushMem(&output_tensors[i].sysMem[0], HB_SYS_MEM_CACHE_INVALIDATE);
	}
	hbDNNReleaseTask(task_handle);
	
    return Post_Process(output,img);
}

void Lead_Horizon::iniImgProcessor()
{
    hasInited=false;
    isDetecting=false;
    onGPU=false;
    save_count = 1;
    onceRunTime=0;

    if(hbDNNInitializeFromFiles(&packed_dnn_handle, &model_name, 1))
        return;
    const char **model_name_list;
    int model_count = 0;
    if(hbDNNGetModelNameList(&model_name_list, &model_count, packed_dnn_handle))
        return;
    if(hbDNNGetModelHandle(&dnn_handle, packed_dnn_handle, model_name_list[0]))
        return;
    if(hbDNNGetInputCount(&input_count, dnn_handle)) return;
    //assert(input_count == 1 && "only input_count 1 is avalid!");
    if(input_count != 1){
        printf("only input_count 1 is avalid!\n");
        return;
    }
    if(hbDNNGetOutputCount(&output_count, dnn_handle)) return ;
    input_tensors.resize(input_count);
    output_tensors.resize(output_count);
    if(prepare_tensor()) return ;
    cout << "Model Loaded! "<<endl;
    pic_size = input_h * input_w * input_c;
    resizer = new BpuResize(rawW, rawH, input_w, input_h,imageType::BGR);
    hasInited=true;
}

void Lead_Horizon::startProcessOnce()
{
    usrtimer.start();
    if (!ipcMutex.tryLock())
        return;
    if(img_input1.channels()==4)
        cvtColor(img_input1,img_output3,COLOR_BGRA2RGB);
    else if(input_is_bayer)
        cvtColor(img_input1,img_output3,COLOR_BayerRG2BGR);
    else
        resize(img_input1,img_output3,Size(input_w,input_h),0,0,INTER_NEAREST);

    if(rawW<4081 && rawH<4081 && !input_is_bayer)
        resizer->Resize(img_output3,img_output3);
    else
        resize(img_output3,img_output3,Size(input_w,input_h),0,0,INTER_NEAREST);
    ipcMutex.unlock();

    if (!img_output3.isContinuous())
        img_output3 = img_output3.clone();
    hbDNNTensor *input = input_tensors.data();
    auto data = input->sysMem[0].virAddr;
    memcpy(reinterpret_cast<uint8_t *>(data), img_output3.data, pic_size);

    cout<<"Time of Post Process(ms): "<<
          usrtimer.elapsed()<<endl;

    hbDNNTaskHandle_t task_handle = nullptr;
    hbDNNTensor *output = output_tensors.data();
    for (int i = 0; i < input_count; i++) {
        hbSysFlushMem(&input_tensors[i].sysMem[0], HB_SYS_MEM_CACHE_CLEAN);
    }
    hbDNNInferCtrlParam infer_ctrl_param;
    HB_DNN_INITIALIZE_INFER_CTRL_PARAM(&infer_ctrl_param);
    if(hbDNNInfer(&task_handle,&output,input_tensors.data(),dnn_handle,&infer_ctrl_param))
        return;
    if(hbDNNWaitTaskDone(task_handle, 0))
        return;
    for (int i = 0; i < output_count; i++) {
        hbSysFlushMem(&output_tensors[i].sysMem[0], HB_SYS_MEM_CACHE_INVALIDATE);
    }
    hbDNNReleaseTask(task_handle);

    cout<<"Time of Inference(ms): "<<
          usrtimer.elapsed()<<endl;

    hbSysFlushMem(&(output->sysMem[0]), HB_SYS_MEM_CACHE_INVALIDATE);
    uint64 * pred = (uint64 *)(output->sysMem[0].virAddr);

    img_output2 = Mat(outputH,outputW,CV_8UC(8),pred);
    extractChannel(img_output2,img_output2,0);
    resize(img_output3,img_output1,Size(outputW,outputH),INTER_NEAREST);

//    resize(img_output2,img_output2,Size(rawW,rawH),INTER_NEAREST);
//    resize(img_output3,img_output1,Size(rawW,rawH),INTER_NEAREST);

    for (int i = 1; i < classNums; i++)
    {
        img_output2.copyTo(img_output3);
        threshold(img_output3,img_output3,i-1,255,ThresholdTypes(THRESH_TOZERO));
        threshold(img_output3,img_output3,i,255,ThresholdTypes(THRESH_TOZERO_INV));
        threshold(img_output3,img_output3,i-1,255,ThresholdTypes(THRESH_BINARY));

        vector<vector<Point>> cnts;
        findContours(img_output3,cnts,RetrievalModes(RETR_LIST),CHAIN_APPROX_SIMPLE);
        drawContours(img_output1,cnts,-1,Scalar(i*50,255-i*50,175-i*25),2);
    }

    cout<<"Time of After Process(ms): "<<
          usrtimer.elapsed()<<endl;

    emit outputImgProcessedRequest();

    //    cout<<"Time of Once Run(ms): "<<
    //        usrtimer.elapsed()<<endl;
    onceRunTime=usrtimer.elapsed();
}

int32_t Lead_Horizon::prepare_tensor(){
	hbDNNTensor *input = input_tensors.data();
	for (int i = 0; i < input_count; i++) {
		if(hbDNNGetInputTensorProperties(&input[i].properties, dnn_handle, i))
			return 1;
		int input_memSize = input[i].properties.alignedByteSize;
		if(hbSysAllocCachedMem(&input[i].sysMem[0], input_memSize))
			return 1;
		input[i].properties.alignedShape = input[i].properties.validShape;
		input_h = input[i].properties.validShape.dimensionSize[1];
		input_w = input[i].properties.validShape.dimensionSize[2];
		input_c = input[i].properties.validShape.dimensionSize[3];
		if (input[i].properties.tensorLayout == HB_DNN_LAYOUT_NCHW) {
			input_c = input[i].properties.validShape.dimensionSize[1];
			input_h = input[i].properties.validShape.dimensionSize[2];
			input_w = input[i].properties.validShape.dimensionSize[3];
		}
	}

	hbDNNTensor *output = output_tensors.data();
	for (int i = 0; i < output_count; i++) {
		if(hbDNNGetOutputTensorProperties(&output[i].properties, dnn_handle, i))
			return 1;
		int output_memSize = output[i].properties.alignedByteSize;
		if(hbSysAllocCachedMem(&output[i].sysMem[0], output_memSize))
			return 1;
	}

    outputH = output_tensors[0].properties.validShape.dimensionSize[1];
    outputW = output_tensors[0].properties.validShape.dimensionSize[2];
    if (output_tensors[0].properties.tensorLayout == HB_DNN_LAYOUT_NCHW) {
        classNums= output_tensors[0].properties.validShape.dimensionSize[1]+1;
        outputH = output_tensors[0].properties.validShape.dimensionSize[2];
        outputW = output_tensors[0].properties.validShape.dimensionSize[3];
    }
	return 0;
}

int32_t Lead_Horizon::read_image_2_tensor(float* raw_data){


	
    // memcpy(mat.data, res,2048 * 1024 * 3);
	// cv::resize(img, mat,cv::Size(input_w, input_h));
	// if(img.channels() != input_c){
	// 	printf("input image channels error!\n");
	// 	return 1;
	// }
	if (!mat.isContinuous()) mat = mat.clone();
	hbDNNTensor *input = input_tensors.data();
	// uint8_t *mat_data = mat.ptr<uint8_t>();
	auto data = input->sysMem[0].virAddr;
	// memcpy(reinterpret_cast<uint8_t *>(data), mat_data, pic_size);
    memcpy(reinterpret_cast<uint8_t *>(data), raw_data, pic_size);
	return 0;
}

cv::Mat Lead_Horizon::Post_Process(hbDNNTensor *output,cv::Mat res){
	hbSysFlushMem(&(output->sysMem[0]), HB_SYS_MEM_CACHE_INVALIDATE);
	int64 * pred = reinterpret_cast<int64 *>(output->sysMem[0].virAddr);   
    cv::resize(res,res,cv::Size(512,256)); 
    for(int i = 0; i < 256; i++){
        for(int j = 0; j < 512; j++){
            if(pred[i*512 + j] == 1) {res.at<cv::Vec3b>(i,j) = {0,0,255}; }
        }
   }
    return res;
	
}
