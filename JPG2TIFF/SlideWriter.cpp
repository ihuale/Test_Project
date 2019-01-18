#include "SlideWriter.h"
#include <stdio.h>
#include <ctime>
#include <algorithm>
#include <io.h>//_finddata_t
#include <fstream>//_finddata_t


#include "IniFile.h"


SlideWriter::SlideWriter(string arg_dir, IniFile *arg_ini)
{
	mdir = arg_dir;
	mfilename = arg_dir + "\\slide.jpg";
	mframeNumx = arg_ini->getValueInt("ScanRect", "XFrameNum");
	mframeNumy = arg_ini->getValueInt("ScanRect", "YFrameNum");
	mimageWidth = arg_ini->getValueInt("ScanRect", "ImageWidth");
	mimageHeight = arg_ini->getValueInt("ScanRect", "ImageHeight");
	moffsetX = arg_ini->getValueInt("ScanRect", "XOffset");
	moffsetY = arg_ini->getValueInt("ScanRect", "YOffset");
	mquality = (arg_ini->getValueInt("ScanRect", "Quality") > 50) ? (arg_ini->getValueInt("ScanRect", "Quality")) : 50;

	mMatLines.resize(mframeNumx);
	mRowRemaining = mframeNumx;
	mColRemaining = mframeNumy;
	mNumWorking = 0;

	printf("[SlideWriter]\ninput dir: %s\nframeNumX: %d    frameNumY: %d\noffsetX: %d    offsetY: %d\njpeg quality: %d\n", mdir.c_str(), mframeNumx, mframeNumy, moffsetX, moffsetY, mquality);

	//start to splice
	splice();
}


SlideWriter::~SlideWriter()
{
	if (!mMatLines.size()>0) {
		for (auto &iter : mMatLines) {
			iter.release();
			//iter->release();
		}
	}
	if (!mMatTotal.empty()) {
		mMatTotal.release();
	}
}

bool SlideWriter::getFiles(string arg_dir, vector<string>& arg_files)
{
	long long hFile = 0;

	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(arg_dir).append("\\*.jpg").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
			{
				arg_files.push_back(p.assign(arg_dir).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
	return true;
}

void SlideWriter::splice()
{
	if (mframeNumx*mframeNumy < 1) {
		//in case read ini failed
		std::cout << "[SlideWriter] frame num: " << mframeNumx << "--" << mframeNumy << std::endl;
		return;
	}
	printf("[SlideWriter] start to splice!\n");

	//first,creat tiff file
	int totalWidth = mframeNumx * (mimageWidth -moffsetX) + moffsetX;
	int totalHeight = mframeNumy * (mimageHeight - moffsetY) + moffsetY;
	
	//TODO
	//use multi thread read data
	//then,single thread write to tiff

	//get cpu count
	unsigned int num_cpu = 1;//min is 1
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	num_cpu = si.dwNumberOfProcessors;
	printf("[SlideWriter] The number of cpu detected is: %d\n\n", num_cpu);


	//now, start to read
	//loop through a line of jpg files
	int maxWorkThreadNum = std::min((int)num_cpu, mframeNumx);
	while (mRowRemaining > 0) {
		//creat thread
		if (mMutex.try_lock()) {
			if (mNumWorking < maxWorkThreadNum) {
				//has idle thread
				//allow thread creation
				std::thread temThread(&SlideWriter::readJpgToMat, this, (mframeNumx - mRowRemaining));
				temThread.detach();

				++mNumWorking;
				//--mColRemaining;
				--mRowRemaining;
				printf("[SlideWriter] creat one working thread. remain: %d   ID:%d\n", mRowRemaining,std::this_thread::get_id());
			}
			mMutex.unlock();
		}
	}
	//while (mNumWorking > 0) {
	//	//waitting for all thread over
	//	//printf("[SildeWriter] while working num: %d\n", mNumWorking);
	//}
	bool flag_wait = true;
	while (flag_wait) {
		if (mMutex.try_lock()) {
			if (!(mNumWorking > 0))
				flag_wait = false;
			mMutex.unlock();
		}

	}
	printf("[SlideWriter] all process thread over! waitting for concat......\n");

	std::vector<cv::Mat>::iterator iter = mMatLines.begin();
	mMatTotal = mMatLines[0];
	++iter;
	for (; iter != mMatLines.end(); ++iter) {
		//cv::vconcat(mMatTotal, *iter, mMatTotal);
		cv::hconcat(mMatTotal, *iter, mMatTotal);
		iter->release();
	}

	//now,all over, save the mat to jpg file
	save();

	printf("[SildeWiriter] all over!\n");
}

void SlideWriter::readJpgToMat(int arg_tile_row)
{
	printf("[SlideWriter] now, get the line: %d, total is : %d  ID: %d\n", arg_tile_row, mframeNumx, std::this_thread::get_id());
	//read all arg_tile_tile_j.jpg to tif
	for (int j = 0; j < mframeNumy; ++j) {
		//read image first
		string imgName = mdir + "\\Images\\" + to_string(j) + "_" + to_string(arg_tile_row) + ".jpg";
		auto img = cv::imread(imgName);
		if (img.empty()) {
			printf("[SildeWriter] read img failed: %s  ID: %d\n", imgName.c_str(), std::this_thread::get_id());

			throw runtime_error((string("[SildeWriter] read img failed: ") + imgName).c_str());//splice failed, no need to continue
		}

		cv::Range rWidth, rHeight;

		rWidth.start = 0;
		rHeight.start = 0;

		//last col no need cut the final cols
		//last row no need cut the final rows
		//cv::Mat::size() return is: (height, width)
		//row first
		/*if (j == (mframeNumx - 1)) {
			rWidth.end = img.size().width;
		}
		else {
			rWidth.end = img.size().width - moffsetX;
		}
		if (arg_tile_row == (mframeNumy - 1)) {
			rHeight.end = img.size().height;
		}
		else {
			rHeight.end = img.size().height - moffsetY;
		}*/
		//col first
		if (arg_tile_row == (mframeNumx - 1)) {
			rWidth.end = img.size().width;
		}
		else {
			rWidth.end = img.size().width - moffsetX;
		}
		if (j == (mframeNumy - 1)) {
			rHeight.end = img.size().height;
		}
		else {
			rHeight.end = img.size().height - moffsetY;
		}
		
		auto mask = cv::Mat::Mat(img, rHeight, rWidth);
		//splicing the mask to the back of mMatLine
		mMutex.lock();
		if (0 == j) {
			mMatLines[arg_tile_row] = mask.clone();
		}
		else {
			cv::vconcat(mMatLines[arg_tile_row], mask, mMatLines[arg_tile_row]);
		}
		mMutex.unlock();
		mask.release();
	}
	//printf("[SlideWriter] last time total mat size is: %d\n", mMatTotal.size());
	printf("[SlideWriter] get the line: %d over.  ID: %d\n", arg_tile_row, std::this_thread::get_id());

	//one thread idling
	mMutex.lock();
	--mNumWorking;
	printf("[SlideWriter] one thread over!\tremain thread count: %d  ID: %d\n", mNumWorking, std::this_thread::get_id());
	mMutex.unlock();
}

void SlideWriter::save()
{
	printf("[SldieWriter] start to save\n");

	time_t time_start = clock();

	auto flag = cv::imwrite(mfilename, mMatTotal);

	printf("[SlideWriter] save jpg %s\n", flag ? "true" : "false");

	time_t time_end = clock();

	printf("[SlideWrite] save cost time is: %f  ms\n", difftime(time_end, time_start));

	int temsize = mMatLines.size();
#pragma omp for
	for (int i = 0; i < temsize; ++i) {
		mMatLines[i].release();
		//mMatLines[i]->release();
	}
	mMatTotal.release();
}
