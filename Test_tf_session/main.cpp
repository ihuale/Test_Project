#include <vector>
#include <string>
#include <iostream>
#include <memory>

#include "AIType.h"
#include "QueueBuffer.hpp"
#include "IniFile.h"
#include "OpenSlideImage.h"
#include "RenderThread.h"
#include "function.hpp"

int main(int argc, char*argv[])
{
	//first,get config
	std::string config_filename;
	bool flag_get_filename = false;
	if (argc < 2) {
		while (!flag_get_filename) {
			std::cout << "[Warning] please input the config file path:\n";
			std::cin >> config_filename;
			FILE *fp;
			fp = fopen(config_filename.c_str(), "r");
			if (!fp) {
				//file does not exits
				printf("[ERROR] open file failed!\n");
				flag_get_filename = false;
			}
			else {
				flag_get_filename = true;
			}
			fclose(fp);
		}
	}
	else {
		config_filename = argv[1];
		printf("input file is: %s\n", argv[1]);
	}
	
	//now,have a config file
	//read the config
	IniFile ini;
	ini.setFileName(config_filename);

	auto flag = init(&ini);
	if (!flag) {
		printf("[Main] init failed!\n");
		system("pause");
		return -1;
	}
	//first,preating run
	auto flagRun = preatingRun();
	if (!flagRun) {
		printf("[Main] preating run failed!\n");
		system("pause");
		return -2;
	}

	//init openslide img
	OpenSlideImageSP img = std::make_shared<OpenSlideImage>();
	auto falg_img = img->initializeType(ini.getValueStr("ImgConfig", "img_path"));
	if (!falg_img) {
		printf("[Main] init openslide img failed!\n");
		system("pause");
		return -3;
	}

	//now, creat producer and tfworker
	RenderThread thread(img, imgReaders);
	thread.imgBuffer = &queBuffer;

	//get job
	splice(&thread.m_jobList, img);
	queBuffer.setSize(totalFrams);

	printf("[Main] get job over, reader thread start!\n");
	thread.start();
	processing(&thread.totalFrams);
	thread.stop();
	printf("[Main] all job have done\n");

	//now,creat multi tfworker
	runningWorkers = tfWorkers;
	int temTotalFrams = totalFrams;

	for (int i = 0; i < tfWorkers; ++i) {
		std::thread tem_thread(&sessionRun);
		tem_thread.detach();
	}

	printf("[Main] total frams is: %d\n", temTotalFrams);
	processing(&totalFrams);
	printf("[Main] all worker finished!\n");


	system("pause");
	return 0;
}