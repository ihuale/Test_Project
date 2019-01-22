#include <stdio.h>//for rename
#include <iostream>//for cout
#include <string>
#include <vector>
#include <mutex>
#include <cmath>//for floor

#include "IniFile.h"
#include "SlideWriter.h"

using namespace std;

std::recursive_mutex mMutex;
struct ImageTile {
	int col;
	int row;
};

std::string dir_prefix;
std::string dir_currentDir;

ImageTile getNewNameNum(string arg_current_name,int arg_framex, int arg_framey) {
	//string num=arg_current_name.substr(-1,)
	char szDrive[_MAX_DRIVE];   //磁盘名
	char szDir[_MAX_DIR];       //路径名
	char szFname[_MAX_FNAME];   //文件名
	char szExt[_MAX_EXT];       //后缀名

	_splitpath(arg_current_name.c_str(), szDrive, szDir, szFname, szExt);

	auto num = atoi(szFname);

	//TODO
	//get new name
	int row = 0, col = 0;

 //	if (0 != ((num / arg_framey) % 2)) {
	//	//odd row
	//	col = ((num%arg_framey) == 0) ? 0 : (num % arg_framey);
	//}
	//else {
	//	//even row
	//	col = arg_framey - (num % arg_framey) - 1;
	//}
	//row = arg_framex - (num / arg_framey) - 1;
	//int newNum = col * arg_framex + row;
	col = arg_framex - num / arg_framey - 1;
	row = arg_framey - num % arg_framey - 1;

	return ImageTile{ col,row };
};

//rename just for convenience
void renameFile(vector<std::string>* arg_files, int arg_framex, int arg_framey) {
	if (arg_files->size() < 1) {
		cout << "[renameFile] no file!\n";
		return;
	}

	for (std::string &iter : (*arg_files)) {
		auto newTileNum = getNewNameNum(iter, arg_framex, arg_framey);

		string newName;
		string strCol = to_string(newTileNum.col);
		string strRow = to_string(newTileNum.row);

		/*if (newTileNum.col < 10) {
			strCol = string("0") + to_string(newTileNum.col);
		}
		if (newTileNum.row < 10) {
			strRow = string("0") + to_string(newTileNum.row);
		}*/
		
		newName = dir_prefix + "\\Images\\" + strCol + "_" + strRow + ".jpg";
		//newName = dir_prefix + "\\Images\\" + strCol + strRow + ".jpg";
		auto flag = rename(iter.c_str(), newName.c_str());

		if (flag) {
			cout << "[renameFile] rename failed: " << iter;
		}
		else {
			cout << "[renameFile] rename successed: " << iter;
		}
		cout << "\tnew name is: " << newName << endl;
	}
	printf("[renameFile] over!\n");
};

int main(int argc,char **argv)
{
	////for test
	//cv::Mat arr(4, 4, CV_8UC3, cv::Scalar(255, 255, 255));
	//std::cout << "[main] before data:" << std::endl;
	//std::cout << arr << std::endl;
	//unsigned char arry_cp[2][4] = { {1,2,3,4}, {5,6,7,8} };
	//cv::Mat arry2(2, 4, CV_8UC3, cv::Scalar(125, 125, 125));

	//memcpy(arr.data+ (2 * 4 * 3 + 1), arry2.data, 2 * 4 * 3);
	//
	//std::cout << std::endl << "[main] after data:" << std::endl;
	//std::cout << arr << std::endl;


	dir_currentDir = argv[0];
	dir_currentDir.replace(dir_currentDir.end() - 13, dir_currentDir.end(), "");
	printf("[Main] current path is: %s\n", dir_currentDir.c_str());

	if (argc > 1) {
		dir_prefix = argv[1];
	}
	else {
		printf("[Main] please input the dir:\n");
		cin >> dir_prefix;
	}
	cout << "[Main] input dir is: " << dir_prefix << endl;

	//read the ini file
	string filename_ini = dir_prefix + "\\results.ini";
	IniFile ini;
	ini.setFileName(filename_ini);

	//get config
	auto flag_rename = ini.getValueStr("ScanRect", "ReName");
	auto flag_splice = ini.getValueStr("ScanRect", "Splice");
	auto flag_pyramid = ini.getValueStr("ScanRect", "Pyramid");
	printf("[Main]\nRename: %s\nSplice: %s\nPyramid: %s\n", (strcmp(flag_rename.c_str(), "true") == 0) ? "true" : "false", (strcmp(flag_splice.c_str(), "true") == 0) ? "true" : "false", (strcmp(flag_pyramid.c_str(), "true") == 0) ? "true" : "false");


	if (strcmp(flag_rename.c_str(), "true") == 0) {
		printf("[Main] start rename!\n");
		int frameNumX = ini.getValueInt("ScanRect", "XFrameNum");
		int frameNumY = ini.getValueInt("ScanRect", "YFrameNum");
		vector<string> list_files;
		auto flag_getfiles = SlideWriter::getFiles(dir_prefix+"\\Images", list_files);

		if (flag_getfiles) {
			renameFile(&list_files, frameNumX, frameNumY);
			ini.setValue("ScanRect", "ReName", "false");
		}
		else {
			printf("[Main] no files!\n");
			return -1;
		}
	}

	
	if (strcmp(flag_splice.c_str(), "true") == 0) {
		printf("[Main] start SlideWriter!\n");
		//first, creat SlideWriter
		SlideWriter slide(dir_prefix, &ini);
	}

	
	if (strcmp(flag_splice.c_str(), "true") == 0) {
		printf("[Main] start to creat pyramid file: %s\n", (dir_prefix + "\\Pyramid.tif").c_str());

		int temQuality = (ini.getValueInt("ScanRect", "Quality") > 50) ? (ini.getValueInt("ScanRect", "Quality")) : 50;
		string str_command = dir_currentDir + "\\vips.exe im_vips2tiff ";
		str_command = str_command + dir_prefix + "\\slide.jpg " + dir_prefix + "\\pyramid.tif:jpeg:" + to_string(temQuality).c_str() + ",tile:256x256,pyramid";
		system(str_command.c_str());
		printf("\n\n");
	}


	system("pause");
	return 0;
}