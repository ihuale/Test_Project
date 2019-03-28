#pragma once
#include "AIType.h"
#include "TFIncludeh.h"

namespace ImgPreProcess {

	void config(int w, int h, int ch);

	//tool function
	//covert cv::Mat to tensorflow::Tensor
	//this W means wfirst on config
	Status cvtMat2TensorW(const TFCVMetaPtrVec& argMatList, Tensor& outTensor);

	//this H means hfirst on config
	Status cvtMat2TensorH(const TFCVMetaPtrVec& argMatList, Tensor& outTensor);

}//namesapce ImgPreProcess