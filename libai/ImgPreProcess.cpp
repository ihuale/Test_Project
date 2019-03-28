#include "ImgPreProcess.h"
#include "TF.h"

namespace ImgPreProcess {

	int imgW = 0;
	int imgH = 0;
	int imgCH = 0;

	void config(int w, int h, int ch)
	{
		imgW = w;
		imgH = h;
		imgCH = ch;
	}

	Status cvtMat2TensorW(const TFCVMetaPtrVec& argMatList, Tensor& outTensor)
	{
		//this for cheng-model
		if (argMatList.empty()) {
			return Status(tensorflow::error::Code::INVALID_ARGUMENT,
				"[ImgPreProcess] argMat->img is empty!\n");
		}
		//clear outTensor first???

		time_t time_start = clock();

		int temBatchSize = argMatList.size();
		tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
			tensorflow::TensorShape({ 
				temBatchSize, imgH, imgW, imgCH }));

		auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();

		for (int i = 0; i < temBatchSize; ++i) {
			for (unsigned int th = 0; th < imgH; ++th) {
				auto rowPointer = argMatList[i]->img.ptr(th);
				for (unsigned int tw = 0; tw < imgW; ++tw) {
					res_tensor_mapped(i, th, tw, 2) = (float)(*rowPointer);
					res_tensor_mapped(i, th, tw, 1) = (float)(*(rowPointer + 1));
					res_tensor_mapped(i, th, tw, 0) = (float)(*(rowPointer + 2));

					rowPointer += 3;
				}
			}
		}

		outTensor.CopyFrom(tem_tensor_res, tem_tensor_res.shape());

#ifdef _WS_LOG_
		printf("[ImgPreProcess] cvtMat2TensorW(list): %f\n",
			difftime(clock(), time_start));
#endif // _WS_LOG_

		return Status::OK();
	}

	Status cvtMat2TensorH(const TFCVMetaPtrVec & argMatList, Tensor & outTensor)
	{
		//now, this just for kht-model
		if (argMatList.empty()) {
			return Status(tensorflow::error::Code::INVALID_ARGUMENT,
				"[ImgPreProcess] argMat->img is empty!\n");
		}
		//clear outTensor first???

		time_t time_start = clock();

		int temBatchSize = argMatList.size();
		tensorflow::Tensor tem_tensor_res(tensorflow::DataType::DT_FLOAT,
			tensorflow::TensorShape({
				temBatchSize, imgW, imgH, imgCH }));

		auto res_tensor_mapped = tem_tensor_res.tensor<float, 4>();

		for (int i = 0; i < temBatchSize; ++i) {
			for (unsigned int th = 0; th < imgH; ++th) {
				auto rowPointer = argMatList[i]->img.ptr(th);
				for (unsigned int tw = 0; tw < imgW; ++tw) {
					res_tensor_mapped(i, tw, th, 2) = (float)(*rowPointer);
					res_tensor_mapped(i, tw, th, 1) = (float)(*(rowPointer + 1));
					res_tensor_mapped(i, tw, th, 0) = (float)(*(rowPointer + 2));

					rowPointer += 3;
				}
			}
		}

		outTensor.CopyFrom(tem_tensor_res, tem_tensor_res.shape());

#ifdef _WS_LOG_
		printf("[ImgPreProcess] cvtMat2TensorH(list): %f\n",
			difftime(clock(), time_start));
#endif // _WS_LOG_

		return Status::OK();
	}

}//namesapce ImgPreProcess