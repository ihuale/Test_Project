#pragma once

#include <vector>
#include <list>
#include <string>
#include <iostream>

#include <opencv2/opencv.hpp>

#ifndef __TF_TYPES__
#define __TF_TYPES__

/*************for tensorflow****************/

typedef unsigned int ImageID;

struct ImgDataMeta
{
	//ImageID index;
	unsigned int posX;
	unsigned int posY;
	int level;

	unsigned int width;
	unsigned int height;
	unsigned char* data;
};

typedef struct _image_meta_ {
	int index;
	cv::Mat img;
}ImageCvMeta;

typedef struct _tf_batch_ {
	int index;

	//std::vector<unsigned char*> batch;
	std::vector<cv::Mat> batch;//mat test first
}TF_Batch;

struct TFDataBatch {
	int posX;
	int posY;

	//std::vector<unsigned char*> batch;
	std::vector<ImgDataMeta*> batch;//mat test first
};

class TF_RES {
public:
	double score;
	int index;

	int posX;
	int posY;

	bool operator<(const TF_RES& rt) {
		return this->score > rt.score;
	};
};
typedef std::list<TF_RES> TfResultList;
//typedef std::map<int, TF_RES> TfResultMap;//<index,TF_RES>
typedef std::map<std::string, TF_RES> TfResultMap;

/*************for tensorflow****************/

#endif //!__TF_TYPES__