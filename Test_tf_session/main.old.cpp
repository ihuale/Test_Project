//#ifndef COMPILER_MSVC
//#define COMPILER_MSVC
//#endif //COMPILER_MSVC
//
//#ifndef NOMINMAX
//#define NOMINMAX
//#endif //NOMINMAX
//
//#include <numeric>
//#include <fstream>
//#include <utility>
//#include <vector>
//#include <list>
//#include <queue>
//#include <string>
//#include <fstream>
//#include <iostream>
//#include <regex>
//#include <memory>
//#include <mutex>
//
//#include <eigen/Dense>
//#include "tensorflow/cc/ops/standard_ops.h"
//#include "tensorflow/core/framework/graph.pb.h"
//#include "tensorflow/core/framework/tensor.h"
//#include "tensorflow/core/graph/graph_def_builder.h"
//#include "tensorflow/core/lib/core/errors.h"
//#include "tensorflow/core/lib/io/path.h"
//#include "tensorflow/core/platform/env.h"
//#include "tensorflow/core/platform/logging.h"
//#include "tensorflow/core/public/session.h"
//#include "tensorflow/core/public/session_options.h"
//
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/hal/interface.h>
//
//#include "AIType.h"
//#include "IniFile.h"
//
//using namespace tensorflow;
//
//#define _TF_WORKERS_ 4
//
///////////////////////////////////////////////
////multi thread ancess
//std::unique_ptr<tensorflow::Session> session;
//std::queue<ImageCvMeta> queIn;
//TfResultList ltOut;
//std::mutex mtxIn;
//std::mutex mtxOut;
//
//volatile int totalFrams;
//volatile int runningWorkers;
///////////////////////////////////////////////
//
//
////tf graph congif
//std::string GRAPH;
//std::string opsInput;
//std::vector<std::string> opsOutput;
//
//tensorflow::Tensor tensorInput;
//tensorflow::TensorShape shapeInput;
//tensorflow::SessionOptions* sessionOptions;
//
//bool initTF();
//void configTF();
//bool loadGraph(const std::string& arg_filename);
//
//Status readTensorFromMulMat(const TF_Batch* arg_list_mat, Tensor *outTensor);
//bool splice(const ImageCvMeta&, TF_Batch*);
//
////only one producer thread
//void readImgToQue(const std::string& arg_path);
//
////multi tfworker
//void sessionRun();
//void tfResEnque(std::vector<tensorflow::Tensor>* arg_res, TF_Batch* arg_batch);
//
//int main(int argc, char*argv[])
//{
//	auto flag = initTF();
//	if (!flag) {
//		printf("[main] initTF failed!\n");
//		return -1;
//	}
//	//now, creat producer and tfworker
//	std::string imgPath = "E:\\WSSCan_data\\data\\2018-10-14-200818-972\\";
//	std::thread producer_thread(&readImgToQue, imgPath);
//	producer_thread.detach();
//
//	//now,creat multi tfworker
//	runningWorkers = _TF_WORKERS_;
//	for (int i = 0; i < _TF_WORKERS_; ++i) {
//		std::thread tem_thread(&sessionRun);
//		tem_thread.detach();
//	}
//	while (runningWorkers > 0) {
//		//waitting
//	}
//	printf("[main] all worker finished!\n");
//
//
//	system("pause");
//	return 0;
//}
//
//bool initTF()
//{
//	GRAPH = ".\\model\\Epoch_995_model.pb";
//	opsInput = "input_1:0";
//	opsOutput = { "dense_2/Sigmoid:0" };
//
//	configTF();
//	auto flag = loadGraph(GRAPH);
//
//	return flag;
//}
//
//void configTF()
//{
//	sessionOptions = new tensorflow::SessionOptions;
//	//sessionOptions->config.set_log_device_placement(true);
//	sessionOptions->config.mutable_gpu_options()->set_allow_growth(true);
//	sessionOptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
//	sessionOptions->config.mutable_device_count()->insert({ "GPU",1 });
//}
//
//bool loadGraph(const std::string & arg_filename)
//{
//	tensorflow::GraphDef graph_def;
//	Status load_graph_status =
//		ReadBinaryProto(tensorflow::Env::Default(),
//			arg_filename,
//			&graph_def);
//	if (!load_graph_status.ok()) {
//		printf("[LoadGraph] load graph failed!\n");
//		return false;
//	}
//	session.reset(tensorflow::NewSession(*sessionOptions));
//	auto status_creat_session = session.get()->Create(graph_def);
//	if (!status_creat_session.ok()) {
//		printf("[LoadGraph] creat session failed!\n");
//		return false;
//	}
//	return true;
//}
//
//Status readTensorFromMulMat(const TF_Batch* arg_list_mat, Tensor *outTensor)
//{
//	if (arg_list_mat->batch.size() < 1)
//		return Status(tensorflow::error::Code::INVALID_ARGUMENT,
//			"arg_list_mat->batch.size() < 1");
//
//	time_t time_start, time_end;
//	time_start = clock();
//	int tem_size = arg_list_mat->batch.size();
//	tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
//		tensorflow::TensorShape(
//			{
//				tem_size,
//				arg_list_mat->batch[0].cols,
//				arg_list_mat->batch[0].rows,
//				arg_list_mat->batch[0].channels()
//			}));
//	auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();
//
//	//TODO
//	/*auto tem_pointer = res_tensor_mapped.data();
//	for (auto &tem_mat : (arg_list_mat->batch)) {
//		cv::Mat newSrc(tem_mat.size(), CV_MAKE_TYPE(tem_mat.type(), 4));
//		int from_to[] = { 0,0, 1,1, 2,2, -1,3 };
//		cv::mixChannels(&tem_mat, 1, &newSrc, 1, from_to, 4);
//		memcpy(tem_pointer, newSrc.data, newSrc.cols*newSrc.rows * 4);
//	}*/
//
//	unsigned int tem_batch_index = 0;//current image index
//	for (auto &tem_mat : (arg_list_mat->batch)) {
//		if (tem_batch_index >= tem_size)
//			break;
//
//		for (unsigned int index_height = 0; index_height < tem_mat.rows; ++index_height) {
//			for (unsigned int index_width = 0; index_width < tem_mat.cols; ++index_width) {
//				for (unsigned int index_channels = 0; index_channels < tem_mat.channels(); ++index_channels) {
//					float v = tem_mat.at<cv::Vec3b>(index_height, index_width)[index_channels];
//					res_tensor_mapped(tem_batch_index, index_height, index_width, index_channels) = v / 255.0*2.0 - 1.0;
//				}
//			}
//		}
//		++tem_batch_index;
//	}
//	outTensor->CopyFrom(tem_tensor_res,
//		tensorflow::TensorShape({ tem_size,  arg_list_mat->batch[0].rows, arg_list_mat->batch[0].cols,arg_list_mat->batch[0].channels() }));
//
//	time_end = clock();
//	printf("[TFWorker] readTensorMulFromMat(list): %f\n", difftime(time_end, time_start));
//
//	return Status::OK();
//}
//
//bool splice(const ImageCvMeta &arg_img, TF_Batch *arg_out_batch)
//{
//	if (arg_img.img.empty()) {
//		return false;
//	}
//	//time_t time_st = clock();
//
//	//first,splice to 854 * 854
//	//then,scal to 512 * 512
//	//at last,input to tf queue
//	arg_out_batch->index = arg_img.index;
//
//	//splice
//	int act_w = 854 - 427;
//	int act_h = 854 - 427;
//	int num_w = std::ceil(arg_img.img.cols / (float)act_w);
//	int num_h = std::ceil(arg_img.img.rows / (float)act_h);
//
//	int offset_w = num_w * act_w - arg_img.img.cols;
//	int offset_h = num_h * act_h - arg_img.img.rows;
//
//	cv::Mat tem_img = cv::Mat(arg_img.img.rows + offset_h, arg_img.img.cols + offset_w, CV_8UC3);
//	memcpy(tem_img.data, arg_img.img.data, arg_img.img.cols*arg_img.img.rows*arg_img.img.channels());
//
//	std::pair<int, int> pos_st = { 0,0 };//<w,h>
//
//	//omp here???
//	//get single splice data
//	for (int i = 0; i < num_w - 1; ++i) {
//		pos_st.second = 0;
//		for (int j = 0; j < num_h - 1; ++j) {
//			auto rect_roi = cv::Rect(pos_st.first, pos_st.second, 854, 854);
//			auto tem_splice = cv::Mat(tem_img, rect_roi);
//
//			cv::resize(tem_splice, tem_splice, cv::Size(512, 512));
//
//			arg_out_batch->batch.push_back(tem_splice.clone());
//			tem_splice.release();
//
//			//get next pos y
//			pos_st.second = (pos_st.second + act_w * (j + 1)) % tem_img.cols;
//		}
//		//get next pos x
//		pos_st.first = (pos_st.first + act_h * (i + 1)) % tem_img.rows;
//	}
//	tem_img.release();
//
//	//time_t time_end = clock();
//
//	return true;
//}
//
//void readImgToQue(const std::string & arg_path)
//{
//	if (arg_path.empty()) {
//		printf("[readImgToQue] path is empty: %s\n", arg_path.c_str());
//		return;
//	}
//	IniFile ini;
//	ini.setFileName(arg_path+"\\results.ini");
//
//	int imgxNums = ini.getValueInt("ScanRect","XFrameNum");
//	int imgyNums = ini.getValueInt("ScanRect", "YFrameNum");
//	int totalImgNums = imgxNums * imgyNums;
//
//	mtxIn.lock();
//	totalFrams = totalImgNums;
//	mtxIn.unlock();
//
//	printf("[readImgToQue] start to read img from %s , image number is %d\n", arg_path.c_str(), totalImgNums);
//
//#pragma omp parallel for
//	for (int i = 0; i < totalImgNums; ++i) {
//		std::string filename = arg_path + "\\Images\\" + std::to_string(i) + ".jpg";
//		auto img = cv::imread(filename);
//		if (img.empty()) {
//			printf("[readImgToQue] read img failed: %s\n", filename.c_str());
//			continue;
//		}
//
//		ImageCvMeta temMeta = { i,img.clone() };
//		mtxIn.lock();
//		queIn.push(temMeta);
//		mtxIn.unlock();
//
//		img.release();
//		//printf("[readImgToQue] img read: %s\n", filename.c_str());
//	}
//
//	printf("[readImgQue] all img read over!\n");
//}
//
//void sessionRun()
//{
//	printf("[sessionRun] one sessionRun thread created!\n");
//
//	auto shapeInput = tensorflow::TensorShape();
//	//shapeInput.AddDim(arg_in_list->batch.size());//here
//	//shapeInput.AddDim((int64)m_image_width);
//	//shapeInput.AddDim((int64)m_image_height);
//	//shapeInput.AddDim(m_image_channels);
//	auto tensorInput = Tensor(tensorflow::DT_FLOAT, shapeInput);
//
//	std::vector<tensorflow::Tensor> resOutputs;
//
//	ImageCvMeta frame;
//	while (totalFrams > 0) {
//
//		if (!mtxIn.try_lock()) {
//			//lock failed
//			std::this_thread::sleep_for(std::chrono::milliseconds(10));
//			continue;
//		}
//		//lock successed
//		if (queIn.size() < 1) {
//			mtxIn.unlock();
//			//que no img
//			std::this_thread::sleep_for(std::chrono::milliseconds(10));
//			continue;
//		}
//		//dequeue
//		frame = queIn.front();
//		queIn.pop();
//		mtxIn.unlock();
//
//		printf("[sessionRun] dequeue one img: %d\n", frame.index);
//
//		//now,tranlate cv::Mat to tensor and run session
//		//first,splice
//		TF_Batch tem_batch;
//		auto flag_splice = splice(frame, &tem_batch);
//		if (!flag_splice) {
//			printf("[sessionRun] splice failed: %d\n", frame.index);
//			frame.img.release();
//			continue;
//		}
//
//		//then,translate
//		auto status_translate = readTensorFromMulMat(&tem_batch, &tensorInput);
//		frame.img.release();
//
//		if (!status_translate.ok()) {
//			printf("[sessionRun] read tensor fram mat failed: %d\n", frame.index);
//			continue;
//		}
//		resOutputs.clear();
//
//		time_t time_run_st = clock();
//		auto status_run = session->Run({ {opsInput,tensorInput} },
//			opsOutput, {}, &resOutputs);
//		if (!status_run.ok()) {
//			printf("[sessionRun] run session failed: %d\n", frame.index);
//			continue;
//		}
//		printf("[SessionRun] session->Run() time is:                %f ms\n", difftime(clock(), time_run_st));
//
//		//add res to list
//		tfResEnque(&resOutputs, &tem_batch);
//
//		mtxIn.lock();
//		--totalFrams;
//		mtxIn.unlock();
//	}
//
//	//this thread is finished
//	mtxOut.lock();
//	--runningWorkers;
//	mtxOut.unlock();
//	printf("[sessionRun] one thread is exit!\n");
//}
//
//void tfResEnque(std::vector<tensorflow::Tensor>* arg_res, TF_Batch * arg_batch)
//{
//	for (std::size_t i = 0; i < arg_res->size(); i++) { // number of output ops
//		auto res_tensor_mapped = (*arg_res)[i].tensor<float, 2>();  // row--batch index; col--result
//
//		TF_RES tem_res;
//		tem_res.index = arg_batch->index;
//		tem_res.score = 0.0000;
//
//		// iterate the batch dimension, tem_index--tem_batch_index
//		for (int tem_index = 0; tem_index < (*arg_res)[i].dim_size(0); ++tem_index) {
//			//result
//			tem_res.score += res_tensor_mapped(tem_index);
//			arg_batch->batch[tem_index].release();
//		}
//
//		mtxOut.lock();
//		ltOut.push_back(tem_res);
//		mtxOut.unlock();
//
//		printf("[tfResEnque] one batch is over: %d\n", arg_batch->index);
//	}
//}
