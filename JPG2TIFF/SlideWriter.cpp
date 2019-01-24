#include "SlideWriter.h"
#include <stdio.h>
#include <ctime>
#include <algorithm>
#include <io.h>//_finddata_t
#include <fstream>//_finddata_t
#include <sstream >//for ostringstream

#include "IniFile.h"

SlideWriter::SlideWriter()
{
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

bool SlideWriter::config(string arg_dir, IniFile * arg_ini)
{
	mdir = arg_dir;
	mfilename = arg_dir + "\\slide.jpg";
	mframeNumx = arg_ini->getValueInt("ScanRect", "XFrameNum");
	mframeNumy = arg_ini->getValueInt("ScanRect", "YFrameNum");
	moffsetX = arg_ini->getValueInt("ScanRect", "XOffset");
	moffsetY = arg_ini->getValueInt("ScanRect", "YOffset");
	mquality = (arg_ini->getValueInt("ScanRect", "Quality") > 50) ? (arg_ini->getValueInt("ScanRect", "Quality")) : 50;
	cv::Mat temImg = cv::imread(arg_dir + "\\Images\\0_0.jpg");
	if (temImg.empty()) {
		/*mimageWidth = arg_ini->getValueInt("ScanRect", "ImageWidth");
		mimageHeight = arg_ini->getValueInt("ScanRect", "ImageHeight");*/
		printf("[SlideWriter] dir files is not enough: %s", arg_dir.c_str());
		return false;
	}
	else {
		mimageWidth = temImg.cols;
		mimageHeight = temImg.rows;
	}

	mMatLines.resize(mframeNumx);
	mRowRemaining = mframeNumx;
	mColRemaining = mframeNumy;
	mNumWorking = 0;

	printf("[SlideWriter]\ninput dir: %s\nframeNumX: %d    frameNumY: %d\noffsetX: %d    offsetY: %d\njpeg quality: %d\n", mdir.c_str(), mframeNumx, mframeNumy, moffsetX, moffsetY, mquality);

	//start to splice
	splice();

	auto flag_savejpg = arg_ini->getValueStr("ScanRect", "SaveJpg");
	if (strcmp(flag_savejpg.c_str(), "true") == 0) {
		printf("[Main] start to save jpg file: %s\n", (arg_dir + "\\slide.jpg").c_str());
		//now,all over, save the mat to jpg file
		save();
		arg_ini->setValue("ScanRect", "SaveJpg", "false");
		printf("[SildeWiriter] save over!\n");
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

	cout << "[SlideWriter] start to application memory, size is: " << totalWidth << " * " << totalHeight << " * 3, please waitting...\n";

	time_t time_application_start = clock();
	mMatTotal = cv::Mat(totalHeight, totalWidth,  CV_8UC3, cv::Scalar(255, 255, 255));
	time_t time_application_end = clock();

	cout << "[SlideWriter]application memory over, time is:  " << difftime(time_application_end, time_application_start) << " ms\n";

	cout << "[SlideWriter] start to application memory, size is: " << mimageWidth - moffsetX << " * " << totalHeight << " * " << mMatLines.size() << ", please waitting...\n";

	time_t time_application2_start = clock();
	std::vector<cv::Mat>::iterator iter = mMatLines.begin();
	for (; iter != mMatLines.end() - 1; ++iter) {
		*iter = cv::Mat( totalHeight, mimageWidth - moffsetX, CV_8UC3, cv::Scalar(255, 255, 255));
	}
	*iter = cv::Mat( totalHeight, mimageWidth, CV_8UC3, cv::Scalar(255, 255, 255));
	time_t time_application2_end = clock();

	cout << "[SlideWriter] application memory over, time is:  " << difftime(time_application2_end, time_application2_start) << " ms\n";
	
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
				//readJpgToMat(mframeNumx - mRowRemaining);
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
	while (mNumWorking > 0) {
		//waitting for all thread over
		printf("[SildeWriter] while working num: %d\n", mNumWorking);
	}
	bool flag_wait = true;
	while (flag_wait) {
		if (mMutex.try_lock()) {
			if (!(mNumWorking > 0))
				flag_wait = false;
			mMutex.unlock();
		}

	}
	printf("[SlideWriter] all process thread over! waitting for concat......\n");

	int col_count = 0;
	int current_col = mMatLines.size();
	for (auto &iter:mMatLines) {
		hconcat(&iter, &mMatTotal, col_count);
		col_count += iter.cols;
		iter.release();
		printf("[SlideWriter] one lines copy to Mat... ramain: %d\n", --current_col);
	}

}

void SlideWriter::readJpgToMat(int arg_tile_row)
{
	printf("[SlideWriter] now, get the line: %d, total is : %d  ID: %d\n", arg_tile_row, mframeNumx, std::this_thread::get_id());

	//read all arg_tile_tile_j.jpg to tif
	int row_count = 0;
	for (int j = 0; j < mframeNumy; ++j) {//
		//read image first
		string imgName = mdir + "\\Images\\" + to_string(arg_tile_row) + "_" + to_string(j) + ".jpg";
		auto img = cv::imread(imgName);
		if (img.empty()) {
			printf("[SildeWriter] read img failed: %s  ID: %d\n", imgName.c_str(), std::this_thread::get_id());

			throw runtime_error((string("[SildeWriter] read img failed: ") + imgName).c_str());//splice failed, no need to continue
		}

		cv::Range rWidth, rHeight;

		rWidth.start = 0;
		rHeight.start = 0;

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
		std::ostringstream ss;
		ss << std::this_thread::get_id();
		//cv::imwrite(ss.str() + "_mask.jpg", mask);
		//splicing the mask to the back of mMatLine
		mMutex.lock(); 
		vconcat(&mask, &mMatLines[arg_tile_row], row_count);
		row_count += mask.rows;
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
	//mMatTotal.release();
}

bool SlideWriter::vconcat(cv::Mat* src, cv::Mat* dest,long arg_start_row)
{
	//src stitched under dest
	if (src == NULL || dest == NULL) {
		printf("[SlideWriter] vconcat error: src || dest is NULL!!!\n");
		return false;
	}
	if (src->cols != dest->cols) {
		printf("[SlideWriter] vconcat error: src.cols(%d) != dest.cols(%d)\n", src->cols, dest->cols);
		return false;
	}

	//get data
	//although tem_row_range is a cv::Mat,
	//but it's pointer to origin address of dest
	////so,we can do things at address to affect dest
	//long long cpsize = static_cast<long long>(src->rows)*static_cast<long long>(src->cols) * 3;
	//auto tem_row_range = dest->rowRange(arg_start_row, arg_start_row + src->rows);
	//memcpy(tem_row_range.data, src->data, cpsize);
	
	int nl = src->cols * src->channels();
	for (int i = arg_start_row; i < arg_start_row + src->rows; ++i) {

		auto inData = src->ptr<uchar>(i - arg_start_row);
		auto outData = dest->ptr<uchar>(i);
		for (int j = 0; j < nl; ++j) {
			*outData++ = *inData++;
		}
	}

	return true;
}

bool SlideWriter::hconcat(cv::Mat * src, cv::Mat * dest, int arg_start_col)
{
	//src on the right side of dest
	if (src == NULL || dest == NULL) {
		printf("[SlideWriter] hconcat error: src || dest is NULL!!!\n");
		return false;
	}
	if (src->rows != dest->rows) {
		printf("[SlideWriter] hconcat error: src.rows(%d) != dest.rows(%d)\n", src->rows, dest->rows);
		return false;
	}
	
	//although tem_col_range is a cv::Mat,
	//but it's pointer to origin address of dest
	//so,we can do things at address to affect dest
	/*long long cpsize = static_cast<long long>(src->rows)*static_cast<long long>(src->cols) * 3;
	auto tem_col_range = dest->colRange(arg_start_col, arg_start_col + src->cols);
	memcpy(tem_col_range.data, src->data, cpsize);*/

	int nl = src->cols * src->channels();
	long long pos = arg_start_col * src->channels();
	for (int i = 0; i < src->rows; ++i) {
		auto inData = src->ptr<uchar>(i);
		auto outData = dest->ptr<uchar>(i) + pos;

		for (int j = 0; j < nl; ++j) {
			*outData++ = *inData++;
		} 
	}

	return true;
}
