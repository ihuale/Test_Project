#include "function.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <windows.h>

#include <vips/vips8>

using namespace std;

//store the piece img data
HL::ImgPair *list_img;

//for multi thread
std::recursive_mutex job_mutex;
std::queue<int> job_que;

volatile int numWorking;
volatile bool flag_writting;
volatile bool flag_read_over;

int config()
{
	auto flag_pos = readPosInfo();
	if (!flag_pos) {
		return -1;
	}

	auto flag_rect = calcPosRect();
	if (!flag_pos) {
		return -2;
	}

	auto flag_offset = calcPosOffset();
	if (!flag_offset) {
		return -3;
	}
	return 0;
}

//[F] means function
bool readPosInfo()
{
	//assume all pos in pos.txt
	cout << "[F] read pos info to memory: " << info.dir_prefix + "\\pos.txt\n";
	std::fstream pos_txt;
	pos_txt.open(info.dir_prefix + "\\pos.txt", std::ios::in);
	if (!pos_txt.is_open()) {
		cout << "[F] read file failed: " << info.dir_prefix + "\\pos.txt\n";
		return false;
	}

	char buffer[256];
	bool flag_xory = false;
	while (!pos_txt.eof()) {
		pos_txt.getline(buffer, 256, '\n');
		if (0 == strcmp(buffer, "Y")) {
			flag_xory = true;
			continue;
		}
		switch(flag_xory) {
		case false:
			info.list_xpos.push_back(std::atoi(buffer) / 2.713846154);
			break;
		case true:
			info.list_ypos.push_back(std::atoi(buffer) / 2.713846154);
			break;
		}
		
	}
	pos_txt.close();

	return (info.list_xpos.size() == info.list_ypos.size());
}

bool calcPosRect()
{
	if (info.list_xpos.size() < 2 || info.list_ypos.size() < 2) {
		cout << "[F] pos info is too little\n";
		return false;
	}

	int minx = info.list_xpos[0]; int miny = info.list_ypos[0];
	int maxx = info.list_xpos[0]; int maxy = info.list_ypos[0];

	for (int i = 1; i < info.list_xpos.size(); ++i) {
		auto temposx = info.list_xpos[i];
		if (minx > temposx) {
			minx = temposx;
		}
		if (maxx < temposx) {
			maxx = temposx;
		}

		auto temposy = info.list_ypos[i];
		if (miny > temposy) {
			miny = temposy;
		}
		if (maxy < temposy) {
			maxy = temposy;
		}
	}

	//+1 for redundancy
	info.w = (maxx - minx) / (info.frameNumX - 1)*(info.frameNumX + 1);
	info.h = (maxy - miny) / (info.frameNumY - 1)*(info.frameNumY + 1);

	//right???
	//not used
	info.maxWidth = info.w / info.frameNumX;
	info.maxHeight = info.h / info.frameNumY;

	return true;
}

bool calcPosOffset()
{
	if (info.list_xpos.size() < 2 || info.list_ypos.size() < 2) {
		cout << "[F] pos info is too little\n";
		return false;
	}
	if (info.list_xpos.size() != info.list_ypos.size()) {
		printf("[F] pos list is not complete!\n");
		return false;
	}

	info.list_offset.push_back(HL::PosPair{ 0,0 });
	auto startx = info.list_xpos[0];
	auto starty = info.list_ypos[0];
	for (int i = 1; i < info.list_xpos.size(); ++i) {
		auto posx = info.list_xpos[i];
		auto posy = info.list_ypos[i];
		//here,change y axis!!!
		info.list_offset.push_back(HL::PosPair{ int(posx - startx),int(starty - posy) });
	}
	
	return true;
}

bool splice()
{
	//first,multi thread read img to memory
	//then,multi thread do cut
	//then,single thread write data to mat
	//last,vips creat pyramid tiff
	int totalFrames = info.frameNumX*info.frameNumY;
	list_img = new HL::ImgPair[totalFrames];
	if (!list_img) {
		printf("[F] allocation memory failed\n");
		return false;
	}
	//get cpu count
	unsigned int num_cpu = 1;//min is 1
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	num_cpu = si.dwNumberOfProcessors;
	printf("[F] The number of cpu detected is: %d\n\n", num_cpu);

	int numImgRemain = totalFrames;
	int maxWorkThreadNum = std::min((int)num_cpu, 10);
	numWorking = 0;
	flag_read_over = false;
	flag_writting = true;

	std::thread tt(&getWriteDataJob);
	tt.detach();

	while (numImgRemain > 0) {
		if (maxWorkThreadNum > numWorking) {
			std::thread temthread(&readImg, totalFrames - numImgRemain);
			temthread.detach();

			--numImgRemain;
			++numWorking;
			printf("[F] creat one thread,working is %d, ImageRemain is %d\n", numWorking, numImgRemain);
		}
	}

	while (numWorking) {
		//wait
		//printf("[F] numWorking is %d\n");
		std::this_thread::sleep_for(100ms);
	}
	flag_read_over = true;

	while (flag_writting) {
		//wait
		//printf("[F] wait for write...\n");
		std::this_thread::sleep_for(100ms);
	}

	if (list_img) {
		delete[] list_img;
	}

	return true;
}

void readImg(int arg_index)
{
	if (arg_index < 0) {
		printf("[F] wrong index: %d", arg_index);
		return;
	}
	std::string imgName = info.dir_prefix + "\\Images\\" + std::to_string(arg_index) + ".jpg";
	auto img = cv::imread(imgName);
	if (img.empty()) {
		printf("[F] read img failed: %s   Thread ID:%d\n", imgName, std::this_thread::get_id());
		return;
	}
	//rotate 180,so we can start from the topleft
	cv::flip(img, img, 0);
	cv::flip(img, img, 1);

	*(list_img + arg_index) = HL::ImgPair{ img.clone(),info.list_offset[arg_index] };
	img.release();

	job_mutex.lock();
	job_que.push(arg_index);
	--numWorking;
	job_mutex.unlock();
}

void getWriteDataJob()
{
	while (flag_writting) {
		if (!job_mutex.try_lock()) {
			continue;
		}
		if (job_que.size() < 1 && flag_read_over) {
			//no job
			flag_writting = false;
			job_mutex.unlock();
			break;
		}
		else if (job_que.size() < 1) {
			job_mutex.unlock();
			continue;
		}
		auto job = job_que.front();
		job_que.pop();
		job_mutex.unlock();
		
		doWriteDataJob(job);
	}

	printf("[F] write data over!\n");
}

void doWriteDataJob(int arg_index)
{
	if (arg_index < 0) {
		printf("[F] Wrong job: %d\n", arg_index);
		return;
	}
	//according to the pos warite data to matTotal
	//find job data first
	//then,write data

	//pointer operation
	auto pointer = list_img + arg_index;

	//cut first
	HL::PosPair topleft = { pointer->pos.x,pointer->pos.y };
	if (pointer->pos.x < 0 || pointer->pos.y < 0) {
		cv::Range lwidth, lheight;
		lwidth.start = 0;
		lheight.start = 0;
		lwidth.end = pointer->img.cols;
		lheight.end = pointer->img.rows;
		if (pointer->pos.x < 0) {
			lwidth.start = std::abs(pointer->pos.x);
			topleft.x = 0;
		}
		if (pointer->pos.y < 0) {
			lheight.start = std::abs(pointer->pos.y);
			topleft.y = 0;
		}

		pointer->img = cv::Mat::Mat(pointer->img, lheight, lwidth);
	}

	for (int i = 0; i < pointer->img.rows; ++i) {
		//the rows of i+topleft.y
		int row = i + topleft.y;
		auto dest = datainfo.matTotal.ptr<unsigned char>(row) + topleft.x*3;
		//get the piece pointer
		auto src = pointer->img.ptr<unsigned char>(i);
		//write data
		long long sz = pointer->img.cols*pointer->img.channels();
		memcpy(dest, src, sz);
	}
	/*if (!pointer->img.empty()) {
		cv::imshow("test", pointer->img);
		cv::waitKey(10000);
	}*/
	pointer->img.release();
	printf("[F] write one img over: %d\n", arg_index);
}

bool pyramid()
{
	printf("[F] pyramid tiff\n");

	//if (!datainfo.matTotal.empty()) {
	//	//for test
	//	cv::resizeWindow("test", 500, 500);
	//	cv::imshow("test", datainfo.matTotal);
	//	cv::waitKey(10000000);
	//}

	int channels = datainfo.matTotal.channels();
	long long totalWidth = datainfo.matTotal.cols;
	long long totalHeight = datainfo.matTotal.rows;

	cout << "[F] start to cvtcolor, size is: " << totalWidth << " * " << totalHeight << " * 3, please waitting...\n";
	cv::cvtColor(datainfo.matTotal, datainfo.matTotal, CV_BGR2RGB);

	cout << "[F] cvtcolor over!\n";

	cout << "[F] start to creat vips image, size is: " << totalWidth << " * " << totalHeight << " * 3, please waitting...\n";

	auto *out = vips_image_new_from_memory(datainfo.matTotal.data,
		channels * totalWidth*totalHeight,
		totalWidth, totalHeight,
		channels, VIPS_FORMAT_UCHAR);

	if (out == NULL) {
		printf("[F] creat VipsImage failed\n");
		return false;
	}

	cout << "[F] creat vips image over!\n";

	cout << "[F] start to vips_tiffsave, size is: " << totalWidth << " * " << totalHeight << " * 3, please waitting...\n";
	
	vips_tiffsave(out, (info.dir_prefix + "\\pyramid.tif").c_str(),
		"compression", VIPS_FOREIGN_TIFF_COMPRESSION_JPEG,
		"Q", 90, "tile", TRUE,
		"tile_width", 256,
		"tile_height", 256,
		"pyramid", TRUE,
		"bigtiff", TRUE,
		NULL);

	cout << "[F] vips_tiffsave over\n";
	g_object_unref(out);
	vips_shutdown();

	//delete[] tem_data;

	printf("[F] creat pyramid tiff over: %s\n", (info.dir_prefix + "\\pyramid.tif").c_str());

	return true;
}
