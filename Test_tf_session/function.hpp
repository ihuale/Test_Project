#pragma once
#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif //COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

#include <numeric>
#include <fstream>
#include <utility>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <memory>
#include <mutex>

#include <eigen/Dense>
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/public/session_options.h"

#include "AIType.h"
#include "QueueBuffer.hpp"
#include "IniFile.h"
#include "OpenSlideImage.h"
#include "RenderThread.h"

using namespace tensorflow;

/////////////////////////////////////////////
//multi thread ancess
std::unique_ptr<tensorflow::Session> session;
std::queue<ImageCvMeta> queIn;
QueueBuffer<ImgDataMeta*> queBuffer;
TfResultMap mapOut;

std::mutex mtxIn;
std::mutex mtxOut;

volatile int totalFrams;
volatile int runningWorkers;

int tfWorkers;
int imgReaders;
/////////////////////////////////////////////

//tf graph congif
std::string GRAPH;
std::string opsInput;
std::vector<std::string> opsOutput;

//tf batch config
int batch_size;
int img_width;
int img_height;
int img_channels;

//for calc
//which level to calc
int level;

tensorflow::Tensor tensorInput;
tensorflow::TensorShape shapeInput;
tensorflow::SessionOptions* sessionOptions;
/////////////////////////////////////////////

bool init(IniFile* argIni);
void configTF(IniFile* argIni);
bool loadGraph(const std::string& arg_filename);

Status readTensorFromMulMat(const TF_Batch* arg_list_mat, Tensor *outTensor);

//multi tfworker
void sessionRun();
void tfResEnque(std::vector<tensorflow::Tensor>* arg_res,
	std::vector<ImgDataMeta*>* arg_batch);

bool preatingRun();

void splice(std::queue<RenderJob>* argList, OpenSlideImageSP argImg);
void processing(volatile int* argNum);

//-------------------------------------------------------------------//

bool init(IniFile* argIni)
{
	GRAPH = argIni->getValueStr("TFConfig", "pb_path");
	opsInput = argIni->getValueStr("TFConfig", "ops_input");
	opsOutput = { argIni->getValueStr("TFConfig","ops_output") };

	tfWorkers = argIni->getValueInt("TFConfig", "tf_workers");
	imgReaders = argIni->getValueInt("ImgConfig", "img_readers");
	level = argIni->getValueInt("ImgConfig", "level");

	batch_size = argIni->getValueInt("TFConfig", "batch_size");
	img_width = argIni->getValueInt("TFConfig", "img_width");
	img_height = argIni->getValueInt("TFConfig", "img_height");
	img_channels = argIni->getValueInt("TFConfig", "img_channels");

	configTF(argIni);

	return loadGraph(GRAPH);
}

void configTF(IniFile* argIni)
{
	sessionOptions = new tensorflow::SessionOptions;
	//sessionOptions->config.set_log_device_placement(true);
	sessionOptions->config.mutable_gpu_options()->set_allow_growth(true);
	sessionOptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
	sessionOptions->config.mutable_device_count()->insert({ "GPU",1 });
}

bool loadGraph(const std::string & arg_filename)
{
	tensorflow::GraphDef graph_def;
	Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(),
			arg_filename,
			&graph_def);
	if (!load_graph_status.ok()) {
		printf("[LoadGraph] load graph failed!\n");
		return false;
	}
	session.reset(tensorflow::NewSession(*sessionOptions));
	auto status_creat_session = session.get()->Create(graph_def);
	if (!status_creat_session.ok()) {
		printf("[LoadGraph] creat session failed!\n");
		return false;
	}
	return true;
}

Status readTensorFromMulMat(const std::vector<ImgDataMeta*>* arg_list_mat, Tensor *outTensor)
{
	if (arg_list_mat->size() < 1)
		return Status(tensorflow::error::Code::INVALID_ARGUMENT,
			"arg_list_mat->batch.size() < 1");

	//time_t time_start, time_end;
	//time_start = clock();
	int tem_size = arg_list_mat->size();
	tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape({ tem_size, img_width, img_height, img_channels }));
	auto resPointer = tem_tensor_res.tensor<float, 4>().data();

	unsigned int tem_batch_index = 0;//current image index
	unsigned int pixels = img_width * img_height*img_channels;
	for (auto &temImg : (*arg_list_mat)) {
		for (int i = 0; i < pixels; ++i) {
			auto temRgb = temImg->data + i;
			*(resPointer + i) = (*temRgb) / 255.0*2.0 - 1.0;
			*(resPointer + i + 1) = (*(temRgb + 1)) / 255.0*2.0 - 1.0;
			*(resPointer + i + 2) = (*(temRgb + 2)) / 255.0*2.0 - 1.0;
		}
		++tem_batch_index;
	}
	outTensor->CopyFrom(tem_tensor_res,
		tensorflow::TensorShape({ tem_size,  img_height, img_width,img_channels }));

	//time_end = clock();
	//printf("[TFWorker] readTensorMulFromMat(list): %f\n", difftime(time_end, time_start));

	return Status::OK();
}

void sessionRun()
{
	//printf("[sessionRun] one sessionRun thread created!\n");
	std::list<TF_RES> temList;
	std::vector<tensorflow::Tensor> resOutputs;

	auto shapeInput = tensorflow::TensorShape();
	auto tensorInput = Tensor(tensorflow::DT_FLOAT, shapeInput);

	//ImageCvMeta frame;
	std::vector<ImgDataMeta*> batch;
	//must delete *frame,in case memory leak
	ImgDataMeta** frame = new ImgDataMeta*;
	while (!queBuffer.isDequeueOver()) {
		//eaque: i<(std::min(batch_size,queBuffer.size()))
		for (int i = 0; i < batch_size && !queBuffer.isDequeueOver(); ++i) {
			bool flag = false;
			*frame = queBuffer.dequeue(&flag);
			if (!flag) {
				//dequeue failed
				//std::this_thread::sleep_for(std::chrono::milliseconds(10));
				--i;
				continue;//for
			}
			else {
				//dequeue successed
				batch.push_back(*frame);
			}
		}
		if (batch.size() < 1) {
			//break;???
			continue;
		}

		//now,tranlate ImgDataMeta to tensor and run session
		auto status_translate = readTensorFromMulMat(&batch, &tensorInput);
		for (auto &iter : batch) {
			if (iter->data) {
				//release data first
				delete[] iter->data;
				iter->data = NULL;
			}
		}

		if (!status_translate.ok()) {
			printf("[sessionRun] read tensor fram mat failed: <%d, %d>\n", (*frame)->posX, (*frame)->posY);
			for (int i = 0; i < batch.size(); ++i) {
				if (batch[i]) {
					delete batch[i];
					batch[i] = NULL;
				}
			}
			batch.clear();
			continue;
		}
		resOutputs.clear();

		time_t time_run_st = clock();
		auto status_run = session->Run({ {opsInput,tensorInput} },
			opsOutput, {}, &resOutputs);
		if (!status_run.ok()) {
			printf("[sessionRun] run session failed: <%d, %d>\n", (*frame)->posX, (*frame)->posY);
			batch.clear();
			for (int i = 0; i < batch.size(); ++i) {
				if (batch[i]) {
					delete batch[i];
					batch[i] = NULL;
				}
			}
			continue;
		}
		mtxIn.lock();
		totalFrams -= batch.size();
		mtxIn.unlock();
		/*printf("[SessionRun] session->Run() time is:       remain fram is: %d         %f ms\n", (totalFrams - 1), difftime(clock(), time_run_st));*/

		//TODO
		//res enqueue when thread will eixt???
		//add res to list
		tfResEnque(&resOutputs, &batch);		
	}

	//this thread is finished
	mtxOut.lock();
	--runningWorkers;
	mtxOut.unlock();

	delete frame;
	frame = NULL;

	//printf("[sessionRun] one thread is exit!\n");
}

void tfResEnque(std::vector<tensorflow::Tensor>* arg_res, std::vector<ImgDataMeta*>* arg_batch)
{
	for (std::size_t i = 0; i < arg_res->size(); i++) { // number of output ops
		auto res_tensor_mapped = (*arg_res)[i].tensor<float, 2>();  // row--batch index; col--result

		TF_RES tem_res;
		tem_res.index = 0;//not used
		tem_res.posX = (*arg_batch)[i]->posX;
		tem_res.posY = (*arg_batch)[i]->posY;
		tem_res.score = res_tensor_mapped(i, 0);
		std::string temID = std::to_string((*arg_batch)[i]->posX)
			+ "_" + std::to_string((*arg_batch)[i]->posY);

		mtxOut.lock();
		mapOut[temID] = tem_res;
		mtxOut.unlock();
		if ((*arg_batch)[i]) {
			delete (*arg_batch)[i];
			(*arg_batch)[i] = NULL;
		}
	}

	arg_batch->clear();
}

bool preatingRun()
{
	printf("[TFSession] start to preating run,please waitting......\n");
	std::vector<ImgDataMeta*> tem_list;

	//enqueue 5 image
	for (int i = 0; i < batch_size; ++i) {
		ImgDataMeta* temMeta = new ImgDataMeta;
		temMeta->posX = 200;
		temMeta->posY = 200;
		temMeta->width = img_width;
		temMeta->height = img_height;
		temMeta->data = new unsigned char[img_width*img_height*img_channels];
		memset(temMeta->data, 255, img_width*img_height*img_channels);

		//release on function exit
		tem_list.push_back(temMeta);
	}

	auto shapeInput = tensorflow::TensorShape();
	shapeInput.AddDim(batch_size);//here
	shapeInput.AddDim(img_width);
	shapeInput.AddDim(img_height);
	shapeInput.AddDim(img_channels);
	auto tensorInput = Tensor(tensorflow::DT_FLOAT, shapeInput);

	Status readTensorStatus = readTensorFromMulMat(&tem_list, &tensorInput);

	if (!readTensorStatus.ok()) {
		LOG(ERROR) << "[TFWorker] Mat->Tensor conversion failed: " << readTensorStatus;
		return false;
	}

	std::vector<tensorflow::Tensor> resOutputs;
	auto status_run = session->Run({ {opsInput,tensorInput} },
		opsOutput, {}, &resOutputs);
	if (!status_run.ok()) {
		LOG(ERROR) << "[TFWorker] Running model failed: " << status_run;
		return false;
	}

	return true;
}

void splice(std::queue<RenderJob>* argQue, OpenSlideImageSP argImg)
{
	if (argQue->size() > 0) {
		std::queue<RenderJob>().swap(*argQue);
	}

	unsigned int imgWidth = argImg->m_dims[0][0];
	unsigned int imgHeight = argImg->m_dims[0][1];

	int numX = std::ceil(imgWidth / (double)img_width) / 2;
	int numY = std::ceil(imgHeight / (double)img_height) / 2;

	totalFrams = numX * numY;

	for (int i = 0; i < numY; ++i) {
		for (int j = 0; j < numX; ++j) {
			RenderJob temJob;
			temJob.imgPosX = j * img_width;
			temJob.imgPosY = i * img_height;
			temJob.width = img_width;
			temJob.height = img_height;
			temJob.level = level;
			argQue->push(temJob);
		}
	}
}

void processing(volatile int* argNum)
{
	time_t time_st = clock();
	if (*argNum < 1) {
		return;
	}
	const int temNum = *argNum;

	printf("[Process] processing, please wait...... .......");
	while (*argNum > 0) {
		double rate = (temNum - (*argNum)) / (double)temNum * 100.;
		if (rate < 10) {
			printf("\b\b\b\b\b\b\b%1.4f%%", rate);
		}
		else {
			printf("\b\b\b\b\b\b\b%2.3f%%", rate);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	auto dfTime = difftime(clock(), time_st);
	printf("\b\b\b\b\b\b\b%3.2f%%\n[Process] process have done\n", 100.0);
	printf("[Process] total time is: %fms, total frame is: %d, fps is: %f\n", dfTime, temNum, (dfTime / (double)temNum * 100));
}
