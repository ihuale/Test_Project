#pragma once

#include <vector>
#include <list>
#include <utility>
#include <string>
#include <iostream>
#include <QPointF>

#include <opencv2/opencv.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern std::string path_image_read;

#ifndef __TF_TYPES__
#define __TF_TYPES__

/*************for tensorflow****************/

typedef unsigned int ImageID;
typedef std::pair<cv::Mat, ImageID> ImageCvMeta;

//ResultMeta: <<iamge, id>,<score, class_id>>
//ScoreMeta: <score, class_id>
typedef std::pair<double, int> ScoreMeta;
typedef std::pair<ImageID, ScoreMeta> ResultMeta;
typedef std::list<ResultMeta> ResultList;
typedef std::list<ResultMeta>::iterator ResultListIter;
//typedef std::map<ImageID, ScoreMeta> ResultMap;

struct BboxInfo {
	//the ceter of bbox center
	QPointF ceter;

	//the width and height of bbox
	int width;
	int height;

	//find name in ClassNameMap with class_id
	int class_id;

	//score for specific color
	double score;

	//the id of parent
	ImageID pid;
};

//bounding box
typedef std::list<BboxInfo> InfoList;
typedef std::list<BboxInfo>::iterator InfoListIter;
//each image has one InfoList
typedef std::map<ImageID, InfoList> InfoMap;//here InfoList or InfoList* ??? //TODO
typedef std::map<ResultMeta, InfoList> UInfoMap;//ultimate InfoMap,this has score meta

/*************for tensorflow****************/

#endif //!__TF_TYPES__