/************************/
//this file for test libai.dll
/************************/
#include <iostream>
#include <stdio.h>
#include <thread>
#include <windows.h>
#include "libai.h"

void rotate90(cv::Mat& img) {
	cv::Mat tmp;
	cv::transpose(img, tmp);
	cv::flip(tmp, img, 1);
};

typedef AIHandle (*AIinitPtr)(AIConfig*, int);
typedef void(*AIfreePtr)(AIHandle);
typedef bool(*AIrunPtr)(AIHandle, TFCVMetaPtrVec&, TFResVec&);

struct Libai {
	HINSTANCE h;
	AIinitPtr init;
	AIfreePtr free;
	AIrunPtr run;
};

Libai* loadLibai(const char* path);
void freeLibai(Libai* p);

int main()
{
	auto pA = loadLibai("libai.dll");

	if (!pA) {
		printf("[Main] load libai failed!\n");
		auto error = GetLastError();
		printf("[Main] last error code: %d\n", error);
		system("pause");
		return -1;
	}
	printf("[Main] libai loader start!\n");

	AIConfig cfg = {
		".\\model\\UNetResNet_500.pb",
		"big_image:0",
		{ "positive_probability:0" },
		1, 1936, 1216, 3, 0, RCOrder::HFIRST, rgbOrder::BGR
	};

	auto handle = pA->init(&cfg, 1);

	if (!handle) {
		printf("[Main] libai loader init libai failed!\n");
		system("pause");
		return -1;
	}

	int imgNum = 8;
	std::string imgPathPrefix = "E:\\Pathology\\test\\111\\Images\\";

	for (int i = 0; i < imgNum; ++i) {
		ImgCVMeta *temMeta = new ImgCVMeta;
		temMeta->index = i;
		temMeta->img = cv::imread(imgPathPrefix + std::to_string(i) + ".jpg");

		//rotate90(temMeta->img);
		std::vector<TF_RES> temRes;
		TFCVMetaPtrVec temVec;
		temVec.push_back(temMeta);
		auto flag = pA->run(handle, temVec, temRes);
	}

	pA->free(handle);

	printf("[Main] libai loader is exit!\n");
	system("pause");
	return 0;
}

Libai* loadLibai(const char* path)
{
	HINSTANCE h = LoadLibraryA(path);

	if (h != NULL)
	{
		Libai* lib = new Libai();
		lib->h = h;
		lib->init = (AIinitPtr)GetProcAddress(h, "AIinit");
		lib->free = (AIfreePtr)GetProcAddress(h, "AIfree");
		lib->run = (AIrunPtr)GetProcAddress(h, "AIrun");

		return lib;
	}

	return nullptr;
}

void freeLibai(Libai * p)
{
	FreeLibrary(p->h);
	delete p;
}
