#pragma once

#include <vector>
#include <list>
#include <string>
#include <iostream>

typedef unsigned int ImageID;

struct BboxInfo {
	struct point {
		int x;
		int y;
	};

	//the ceter of bbox center
	point ceter;

	//the width and height of bbox
	int width;
	int height;

	//find name in ClassNameMap with class_id
	int class_id;

	//score for specific color
	double score;

	//the id of parent
	ImageID pid;

	void print() {
		std::cout << "ImageID: " << pid
			<< " score: " << score
			<< " ceter(" << ceter.x << ", " << ceter.y << ") "
			<< " width: " << width
			<< " height: " << height << std::endl;
	}
};

typedef std::list<BboxInfo> InfoList;
typedef std::list<BboxInfo>::iterator InfoListIter;

//according to ClassNameMap
//0-Normal, 1-LowRatio
//2-MidRatio, 3-HighRatio
const std::vector<std::string> ClassNameMap
		= { {"Normal"},{"LowRatio"},{"MidRatio"},{"HighRatio"} };

const std::vector<double> ClassRatioMap
		= { 0.25, 0.50, 0.75, 1 };//five class

#ifndef _CLASSFICATION__
#define _CLASSFICATION__
	int Classification(double arg_score);
#endif //_CLASSFICATION__