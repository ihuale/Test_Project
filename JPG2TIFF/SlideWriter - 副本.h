#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

extern "C" {
	namespace TIFFLIB {
#include <tiffio.h>;
	}
}

using namespace std;

class IniFile;

typedef unsigned char BYTE;

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

	//function
	void splice();
	void getLineToTiff(int arg_tile_col);

	//config
	bool createFile();
	bool setAttributes(SlideAttributes arg_attributes);

	//tiff fucntion package
	bool writeEncodedTile(BYTE* buff, int x, int y, int z);

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

	TIFFLIB::TIFF * mtif;

	std::ostringstream merrMsg;
};

