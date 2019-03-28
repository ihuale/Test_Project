#include "IniFile.h"
#include "function.h"

HL::SpliceInfo info;
HL::DataInfo datainfo;


int main(int argc, char**argv)
{
	if (argc < 2) {
		std::cout << "[Main] please input dir:\n";
		std::cin >> info.dir_prefix;
		std::cout << "[Main] input dir is: " << info.dir_prefix << std::endl;
	}
	info.dir_prefix = argv[1];
	info.ini.setFileName(info.dir_prefix + "\\results.ini");
	info.frameNumX = info.ini.getValueInt("ScanRect", "XFrameNum");
	info.frameNumY = info.ini.getValueInt("ScanRect", "YFrameNum");

	if (info.frameNumX*info.frameNumY < 2) {
		//0 or 1
		printf("[Main] read config error: XFrameNum--%d  YFrameNum--%d\n", info.frameNumX, info.frameNumY);
		system("pause");
		return -1;
	}

	time_t time_config_start = clock();

	//first,config
	auto flag_config = config();
	if (flag_config) {
		std::string error_function = "";
		switch (flag_config) {
		case -1:
			error_function = "readPosInfo";
			break;
		case -2:
			error_function = "calcPosRect";
			break;
		case -3:
			error_function = "calcPosOffset";
			break;
		}
		printf("[Main] config failed,error function is: %s\n", error_function);
		system("pause");
		return flag_config;
	}

	time_t time_start_allocation = clock();
	//next,allocation memory
	printf("[Main] start to allocation memory,size is: %d * %d * 3\n", info.w, info.h);
	datainfo.matTotal = cv::Mat(info.h, info.w, CV_8UC3, { 255,255,255 });
	//for now,don't think about single lines do splice
	//datainfo.matLines.resize(info.frameNumY);//y first
	//for (auto &iter : datainfo.matLines) {
	//}

	time_t time_start_splice = clock();
	auto flag_splice = splice();
	if (!flag_splice) {
		printf("[Main] splice failed!\n");
		system("pause");
		return -4;
	}

	//write pyramid
	time_t time_start_vips = clock();
	auto flag_pyramid = pyramid();
	if (!flag_pyramid) {
		printf("[Main] pyramid failed!\n");
		system("pause");
		return -5;
	}
	std::cout << "[Main] all over!\n";

	if (!datainfo.matTotal.empty()) {
		datainfo.matTotal.release();
	}
	for (auto &iter : datainfo.matLines) {
		if (!iter.empty()) {
			iter.release();
		}
	}
	system("pause");
	return 0;
}