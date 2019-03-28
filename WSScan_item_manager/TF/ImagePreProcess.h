#pragma once
#include "AIType.h"

#include <vector>
#include <QSize>

extern std::string path_image_read;

#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif //COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

//#include <eigen/Dense>
#include "tensorflow/core/framework/tensor.h"

using tensorflow::Tensor;
using tensorflow::Status;

//this is a tool function 
class ImagePreProcess
{
public:
	virtual ~ImagePreProcess();

	static bool flag_queue;

	//changed size and pixel
	static bool reSample(std::vector<ImageCvMeta> *arg_in_list,
		std::vector<ImageCvMeta> *arg_out_list,
		double arg_in_scale = 0.296, double arg_out_scale = 0.243);

	static bool reSampleAndDig(std::vector<ImageCvMeta> *arg_in_list,
		std::vector<ImageCvMeta> *arg_out_list,
		double arg_in_scale = 0.296, double arg_out_scale = 0.243,
		QSize arg_out_size = QSize(512, 512));

	static bool normlize(std::vector<ImageCvMeta> *arg_in_list);
	static bool normlize(ImageCvMeta *arg_in_mat);

	//convert cv::Mat to tensorflow::Tensor
	static Status cvtMat2Tensor(std::vector<ImageCvMeta>* arg_in_list, Tensor *arg_out);

	static Status mergeMulTensor(std::vector<Tensor>* arg_in_tensor_list, Tensor* arg_out_tensor);

private:
	//prohibit creating objects
	ImagePreProcess();
};