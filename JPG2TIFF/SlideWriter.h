#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#define NOMINMAX
#define G_OS_WIN32
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
	SlideWriter();
	~SlideWriter();

public:
	static bool getFiles(string arg_dir, vector<string> &arg_files) ;

	bool config(string arg_dir, IniFile* arg_ini);
private:
	//function
	void splice();
	void readJpgToMat(int arg_tile_row);
	void save();

	//src stitched under dest
	static bool vconcat(cv::Mat* src, cv::Mat* dest, long arg_start_row);

	//src on the right side of dest
	static bool hconcat(cv::Mat* src, cv::Mat* dest, int arg_start_col);

public:
	int mframeNumx, mframeNumy;
	int mimageWidth, mimageHeight;
	int moffsetX, moffsetY;
	int mquality;

	string mdir;
	string mfilename;
	
	cv::Mat mMatTotal;

private:
	//for multi thread
	int mColRemaining, mRowRemaining;
	int mNumWorking;//working thread

	std::vector<cv::Mat> mMatLines;
};