#pragma once

#include <vector>
#include <list>
#include <string>
#include <map>
#include <utility>
#include <iostream>
#include <QPointF>
#include <opencv2/highgui.hpp>

#include "TF/types.h"

extern std::string path_image_read;

//typedef unsigned int ImageID;
//
//struct BboxInfo {
//	//the ceter of bbox center
//	QPointF ceter;
//
//	//the width and height of bbox
//	int width;
//	int height;
//
//	//find name in ClassNameMap with class_id
//	int class_id;
//
//	//score for specific color
//	double score;
//
//	//the id of parent
//	ImageID pid;
//};
//
//typedef std::list<BboxInfo> InfoList;
//typedef std::list<BboxInfo>::iterator InfoListIter;
//
///*******************************************/
//typedef unsigned int ImageID;
//typedef std::pair<cv::Mat, ImageID> ImageCvMeta;
//
////ResultMeta: <<iamge, id>,<score, class_id>>
////ScoreMeta: <score, class_id>
//typedef std::pair<double, int> ScoreMeta;
//typedef std::pair<ImageID, ScoreMeta> ResultMeta;
////typedef std::pair<ImageID, ScoreMeta> ResultMeta;
//typedef std::map<ImageID, ScoreMeta> ResultMap;
///*******************************************/

//according to ClassNameMap
//0-Normal, 1-LowRatio
//2-MidRatio, 3-HighRatio
const std::vector<std::string> ClassNameMap
= { {"Normal"},{"LowRatio"},{"MidRatio"},{"HighRatio"},{"ERROR"} };

const std::vector<double> ClassRatioMap
		= { 0.25, 0.50, 0.75, 1 };//five class

#ifndef _CLASSFICATION__
#define _CLASSFICATION__
	int Classification(double arg_score);
#endif //_CLASSFICATION__