#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#define NOMINMAX
#include <windows.h>
#include <opencv2/opencv.hpp>

using namespace std;

class IniFile;
extern std::recursive_mutex mMutex;

struct SlideAttributes {
	int newSamplesPerPixel;//0.273um/pixel
	int newBitsPerSample;//8
	int newImageWidth;
	int newImageHeight;
	int newTileWidth;
	int newTileHeight;
	int newTileDepth;//24
	int quality;
};

class SlideWriter
{
	//according to the arg_ini file,
	//creat the pyramid-tiff,
	//use all the image from arg_dir
public:
	SlideWriter(string arg_dir, IniFile* arg_ini);
	~SlideWriter();

public:
	static bool getFiles(string arg_dir, vector<string> &arg_files) ;

private:
	//function
	void splice();
	void readJpgToMat(int arg_tile_row);
	void save();

private:
	int mframeNumx, mframeNumy;
	int mimageWidth, mimageHeight;
	int moffsetX, moffsetY;
	int mquality;

	//tiff attributes
	int mtileWidth, mtileHeight;
	unsigned int mactualWidth, mactualHeight, mbitCount;
	int msamplesPerPixel;

	string mdir;
	string mfilename;

	std::ostringstream merrMsg;

private:
	std::vector<cv::Mat> mMatLines;
	cv::Mat mMatTotal;

	//for multi thread
	int mColRemaining, mRowRemaining;
	int mNumWorking;//working thread
};