#pragma once
#include <stdio.h>//for rename
#include <iostream>//for cout
#include <string>
#include <vector>
#include <mutex>
#include <cmath>//for floor
#include <ctime>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/types_c.h>
#include "IniFile.h"


#ifndef _CUSTOMER_STRUCT_
#define _CUSTOMER_STRUCT_

namespace HL {
	struct PosPair
	{
		//x means cols
		int x;
		//y means rows
		int y;
	};

	struct SpliceInfo
	{
		std::string dir_prefix;
		IniFile ini;
		std::vector<int> list_xpos;
		std::vector<int> list_ypos;

		//read
		int frameNumX, frameNumY;

		//calc
		int w, h;
		int maxWidth, maxHeight;
		std::vector<PosPair> list_offset;
	};

	struct DataInfo
	{
		//for store data
		cv::Mat matTotal;
		std::vector<cv::Mat> matLines;
	};

	struct ImgPair
	{
		cv::Mat img;
		//this pos is the position where img should wait after cropping
		PosPair pos;
	};

}//namespace HL
#endif // !_CUSTOMER_STRUCT_

