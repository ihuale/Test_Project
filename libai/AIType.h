#pragma once
#include <vector>
#include <list>
#include <queue>
#include <opencv2/opencv.hpp>

typedef unsigned long long AIHandle;

//RCOrder and rgbOrder just for input tensor shape
enum RCOrder
{
	//row and col order
	//wfirst means input tensor shape: (batch_size, h, w, ch)
	//hfirst means input tensor shape: (batch_size, w, h, ch)
	WFIRST,
	HFIRST
};

enum rgbOrder
{
	RGB,
	BGR
};

struct AIConfig {
	std::string model_path;
	std::string opsInput;
	std::vector<std::string> opsOutput;
	int batchSize;
	int batchImgW;
	int batchImgH;
	int batchImgChannels;

	//this for clockwise
	int rotate;

	RCOrder pxorder;

	//default is bgr, it's imread order
	rgbOrder rgborder;

	
};

struct ImgCVMeta
{
	int index;
	cv::Mat img;
};

struct TF_RES
{
	int index;
	double score;
};

typedef std::vector<ImgCVMeta> TFCVMetaList;
typedef std::queue<ImgCVMeta> TFCVMetaQue;
typedef std::vector<ImgCVMeta*> TFCVMetaPtrVec;
typedef std::queue<ImgCVMeta*> TFCVMetaPtrQue;
typedef std::list<TF_RES> TFResList;
typedef std::vector<TF_RES> TFResVec;
