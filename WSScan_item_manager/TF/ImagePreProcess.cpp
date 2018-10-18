#include "ImagePreProcess.h"

#ifndef _IMAGEPREPROCESS_INI_
#define _IMAGEPREPROCESS_INI_
bool ImagePreProcess::flag_queue = true;
#endif //_IMAGEPREPROCESS_INI_

ImagePreProcess::~ImagePreProcess()
{
}

bool ImagePreProcess::reSample(std::vector<ImageCvMeta>* arg_in_list, 
	std::vector<ImageCvMeta>* arg_out_list, double arg_in_scale, double arg_out_scale)
{
	if (arg_in_list->size() < 1) {
		return false;
	}
	if ((arg_in_scale <= 0) || (arg_out_scale <= 0)) {
		return false;
	}
	if (arg_out_list->size() > 0) {
		arg_out_list->clear();
	}

	double scale_in_to_out = arg_in_scale / arg_out_scale;
	for (auto &iter : (*arg_in_list)) {
		//resample and convert color space
		auto tem_width = iter.first.rows;
		auto tem_height = iter.first.cols;
		auto tem_image = iter.first.clone();
		tem_image.resize(tem_width*scale_in_to_out, tem_height*scale_in_to_out);

		//TODO
		//warning
		//cv::cvtColor(tem_image, tem_image, CV_RGB2BGR);

		//and then,normalize
		/*tem_image /= (double)255.;
		tem_image -= 0.5;
		tem_image *= 2;*/

		arg_out_list->push_back(std::make_pair(tem_image.clone(), iter.second));

		tem_image.release();
	}

	return true;
}

bool ImagePreProcess::reSampleAndDig(std::vector<ImageCvMeta>* arg_in_list,
	std::vector<ImageCvMeta>* arg_out_list, double arg_in_scale, double arg_out_scale, QSize arg_out_size)
{
	if (arg_in_list->size() < 1) {
		return false;
	}
	if ((arg_in_scale <= 0) || (arg_out_scale <= 0)) {
		return false;
	}
	if ((arg_out_size.width() < 64) || (arg_out_size.height() < 64)) {
		std::cout << "[ImagePreProcess] reSampleAndDig ERROR! size tool small\n";
		return false;
	}
	if (arg_out_list->size() > 0) {
		arg_out_list->clear();
	}

	std::vector<ImageCvMeta> tem_list;
	//first,resample and normalized
	auto flag_resample = reSample(arg_in_list, &tem_list, arg_in_scale, arg_out_scale);
	if (!flag_resample) {
		std::cout << "[ImagePreProcess] reSample failed!\n";
		return false;
	}
	//digging
	for (auto &iter : tem_list) {
		auto tem_width = std::max(0, std::min(iter.first.cols, arg_out_size.width()));
		auto tem_height = std::max(0, std::min(iter.first.rows, arg_out_size.height()));
		auto tem_mat = iter.first(cv::Rect(0, 0, tem_width, tem_height));
		arg_out_list->push_back(std::make_pair(tem_mat.clone(), iter.second));
		tem_mat.release();
	}

	return false;
}

bool ImagePreProcess::normlize(std::vector<ImageCvMeta>* arg_in_list)
{
	if (arg_in_list->size() < 1) {
		return false;
	}
	
	for (auto &iter : (*arg_in_list)) {
		//TODO
		//warning
		//cv::cvtColor(tem_image, tem_image, CV_RGB2BGR);

		//and then,normalize
		iter.first /= (double)255.;
		iter.first -= 0.5;
		iter.first *= 2;

		//arg_out_list->push_back(std::make_pair(tem_image.clone(), iter.second));
	}

	return true;
}

bool ImagePreProcess::normlize(ImageCvMeta * arg_in_mat)
{
	if (arg_in_mat->first.empty()) {
		return false;
	}

	//TODO
	//warning
	//cv::cvtColor(tem_image, tem_image, CV_RGB2BGR);

	//and then,normalize
	arg_in_mat->first /= (double)255.;
	arg_in_mat->first -= 0.5;
	arg_in_mat->first *= 2;

	//arg_out_list->push_back(std::make_pair(tem_image.clone(), iter.second));

	return true;
}

Status ImagePreProcess::cvtMat2Tensor(std::vector<ImageCvMeta> *arg_in_list, Tensor *arg_out)
{
	if (arg_in_list->size() < 1)
		return Status(tensorflow::error::Code::INVALID_ARGUMENT,
			"arg_list_mat.size() < 1");

	int tem_width = (*arg_in_list)[0].first.cols, tem_height = (*arg_in_list)[0].first.rows, tem_channels = 3;
	time_t time_start, time_end;
	time_start = clock();
	int tem_size = arg_in_list->size();
	tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape(
	{
		tem_size,
		tem_height,
		tem_width,
		tem_channels
	}));
	auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();

	unsigned int tem_batch_index = 0;//current image index
									 //auto tem_mat = arg.begin();
	for (auto &tem_mat : (*arg_in_list)) {
		if (tem_batch_index >= tem_size)
			break;

		//cv::cvtColor(tem_mat.first, tem_mat.first, CV_RGB2BGR);

		for (unsigned int index_height = 0; index_height < tem_height; ++index_height) {
			for (unsigned int index_width = 0; index_width < tem_width; ++index_width) {
				for (unsigned int index_channels = 0; index_channels < tem_channels; ++index_channels) {
					float tem_pixel = (float)tem_mat.first.at<cv::Vec3b>(index_height, index_width)[index_channels];
					tem_pixel /= 255.;
					tem_pixel -= 0.5;
					tem_pixel *= 2;
					/*res_tensor_mapped(tem_batch_index, index_height, index_width, index_channels) =
						tem_mat.first.at<cv::Vec3b>(index_height, index_width)[index_channels];*/
					res_tensor_mapped(tem_batch_index,  index_height, index_width, index_channels) = tem_pixel;
				}
			}
		}
		++tem_batch_index;
	}
	arg_out->CopyFrom(tem_tensor_res,
		tensorflow::TensorShape({ tem_size, tem_width, tem_height, tem_channels }));

	time_end = clock();
	std::cout << "[TFWorker] readTensorMulFromMat(list): "
		<< difftime(time_end, time_start) << "ms" << std::endl;

	return Status::OK();
}

Status ImagePreProcess::mergeMulTensor(std::vector<Tensor>* arg_in_tensor_list, Tensor *arg_out_tensor)
{
	/*tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape(
	{
		tem_size,
		tem_width,
		tem_height,
		tem_channels
	}));
	auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();
	arg_out->CopyFrom(tem_tensor_res,
		tensorflow::TensorShape({ tem_size, tem_width, tem_height, tem_channels }));*/
	if (arg_in_tensor_list->size() < 2) {
		std::cout << "[ImagePreProcess] too little input tensor!\n";
		return Status(tensorflow::error::Code::INVALID_ARGUMENT,
			"arg_list_mat.size() < 2");
	}
	auto tem_dims = (*arg_in_tensor_list)[0].dims();
	/*tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
		tensorflow::TensorShape(
	{
		tem_size,
		tem_width,
		tem_height,
		tem_channels
	}));*/


	return Status();
}
