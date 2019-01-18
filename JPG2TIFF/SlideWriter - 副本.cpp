#include "SlideWriter.h"
#include <stdio.h>
#include <io.h>//_finddata_t
#include <fstream>//_finddata_t
#include <opencv2/opencv.hpp>

#include "IniFile.h"

using namespace TIFFLIB;


SlideWriter::SlideWriter(string arg_dir, IniFile *arg_ini):
	mtif(0)
{
	mdir = arg_dir;
	mfilename = arg_dir + "\\silde.tiff";
	mframeNumx = arg_ini->getValueInt("ScanRect", "XFrameNum");
	mframeNumy = arg_ini->getValueInt("ScanRect", "YFrameNum");
	mimageWidth = arg_ini->getValueInt("ScanRect", "ImageWidth");
	mimageHeight = arg_ini->getValueInt("ScanRect", "ImageHeight");
	moffsetX = arg_ini->getValueInt("ScanRect", "XOffset");
	moffsetY = arg_ini->getValueInt("ScanRect", "YOffset");
	mquality = (arg_ini->getValueInt("ScanRect", "Quality") > 50) ? (arg_ini->getValueInt("ScanRect", "Quality")) : 50;

	//start to splice
	splice();
}


SlideWriter::~SlideWriter()
{
	if (mtif) {
		TIFFClose(mtif);
		mtif = 0;
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
		cout << "[SlideWriter] frame num: " << mframeNumx << "--" << mframeNumy << endl;
		return;
	}

	//first,creat tiff file
	createFile();
	int totalWidth = mframeNumx * (mimageWidth -moffsetX) + moffsetX;
	int totalHeight = mframeNumy * (mimageHeight - moffsetY) + moffsetY;
	/*SlideAttributes temAttributes = { 0.273, 8, totalWidth, totalHeight, (mimageWidth - moffsetX), (mimageHeight - moffsetY), 24, 90 };*/
	TIFFSetDirectory(mtif, 0);
	SlideAttributes temAttributes = { 3, 8, totalWidth, totalHeight, 512, 512, 24, 90 };
	setAttributes(temAttributes);

	//TODO
	//use multi thread read data
	//then,single thread write to tiff

	//now, start to read the first line to tiff file
	for (int i = 0; i < mframeNumy; ++i) {
		cout << "[SlideWriter] now, get the line: " << i << endl;
		getLineToTiff(i);
		cout << "[SlideWriter] get the line: " << i << "  over!\n" << endl;
	}

	//now,all over, close the tiff file
	if (mtif) {
		TIFFClose(mtif);
		mtif = 0;
	}
	cout << "[SildeWiriter] all over!\n";
}

void SlideWriter::getLineToTiff(int arg_tile_col)
{
	//read all arg_tile_tile_j.jpg to tif
	for (int j = 0; j < mframeNumx; ++j) {
		//read image first
		string imgName = mdir + "\\Images\\" + to_string(arg_tile_col) + "_" + to_string(j) + ".jpg";
		auto img = cv::imread(imgName);
		if (img.empty()) {
			cout << "[SildeWriter] read img failed: " << imgName << endl;

			throw runtime_error((string("[SildeWriter] read img failed: ") + imgName).c_str());//splice failed, no need to continue
		}

		cv::Range rWidth, rHeight;
		rWidth.start = 0;
		rWidth.end = img.size().width - moffsetX;
		rHeight.start = 0;
		rHeight.end = img.size().height - moffsetY;
		
		auto mask = cv::Mat::Mat(img, rHeight,rWidth);
		/*cv::imshow("img", img);//for test
		cv::imshow("mask", mask);
		cv::waitKey(100000);*/
		auto flag = writeEncodedTile(mask.data, img.size().width - moffsetX, img.size().height - moffsetY, 0);
		if (!flag) {
			cout << "[SildeWriter] write img to tif failed: " << imgName << endl;

			throw runtime_error((string("[SildeWriter] write img to tif failed: ") + imgName).c_str());//splice failed, no need to continue
		}
	}
}

bool SlideWriter::createFile()
{
	if (mtif) {
		TIFFClose(mtif);
		mtif = 0;
	}
	mquality = 70;

	try {
		mtif = TIFFOpen(mfilename.c_str(), "wb8");
		if (!mtif) {
			merrMsg << "Error opening '" << mfilename << "': ";
			throw std::runtime_error(merrMsg.str());
		}
	}
	catch (std::runtime_error)
	{
		if (mtif) TIFFClose(mtif);
		mtif = 0;
		return false;
	}
	cout << "[SlideWriter] creat tiff file succeed: " << mfilename << endl;
	return true;
}

bool SlideWriter::setAttributes(SlideAttributes arg_attributes)
{
	//that should be used at last?
	//the offset is fixed,
	//so if col and row is fixed, the silde w and h is fixed too!!
	mactualWidth = arg_attributes.newImageWidth;
	mactualHeight = arg_attributes.newImageHeight;
	mtileWidth = arg_attributes.newTileWidth;
	mtileHeight = arg_attributes.newTileHeight;
	mbitCount = arg_attributes.newBitsPerSample;
	msamplesPerPixel = arg_attributes.newSamplesPerPixel;
	mquality = arg_attributes.quality;
	uint32 u32TifImageWidth = (uint32)arg_attributes.newImageWidth;
	uint32 u32TifImageLength = (uint32)arg_attributes.newImageHeight;
	uint32 u32TileWidth = (uint32)arg_attributes.newTileWidth;
	uint32 u32TileLength = (uint32)arg_attributes.newTileHeight;
	uint32 u32TileDepth = (uint32)arg_attributes.newTileDepth;//24?
	uint16 u16BitsPerSample = (uint16)arg_attributes.newBitsPerSample;//8?
	uint16 u16SamplesPerPixel = (uint16)arg_attributes.newSamplesPerPixel;//0.273!!!!
	uint16 photometric = PHOTOMETRIC_RGB;
	uint16 planarConfig = 1;

	if (mtif)
	{
		try
		{
			TIFFSetField(mtif, TIFFTAG_IMAGEWIDTH, u32TifImageWidth);
			TIFFSetField(mtif, TIFFTAG_IMAGELENGTH, u32TifImageLength);
			TIFFSetField(mtif, TIFFTAG_BITSPERSAMPLE, u16BitsPerSample);
			TIFFSetField(mtif, TIFFTAG_SAMPLESPERPIXEL, u16SamplesPerPixel);
			//TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);  
			//TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &stripByteCounts);
			TIFFSetField(mtif, TIFFTAG_PHOTOMETRIC, photometric);
			if (mtileWidth > 0 && mtileHeight > 0)
			{
				TIFFSetField(mtif, TIFFTAG_TILEWIDTH, u32TileWidth);//must be a multiple 16
				TIFFSetField(mtif, TIFFTAG_TILELENGTH, u32TileLength);//must be a multiple 16
				TIFFSetField(mtif, TIFFTAG_TILEDEPTH, u32TileDepth);
			}
			TIFFSetField(mtif, TIFFTAG_PLANARCONFIG, planarConfig);
			TIFFSetField(mtif, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
			TIFFSetField(mtif, TIFFTAG_JPEGQUALITY, mquality);//TODO
			TIFFSetField(mtif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);

			TIFFSetField(mtif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE);
		}
		catch (std::bad_alloc)
		{
			if (mtif) TIFFClose(mtif);
			merrMsg << "[SlideWriter] Insufficient memory to decompress '" << mfilename;
			merrMsg << "' into memory";
			mtif = 0;
			return false;
		}
		catch (std::runtime_error)
		{
			if (mtif) TIFFClose(mtif);
			mtif = 0;
			return false;
		}
	}
	else
	{
		return false;
	}

	cout << "[SlideWriter] now, write the attribute over!" << endl;
	return true;
}

bool SlideWriter::writeEncodedTile(BYTE * buff, int x, int y, int z)
{
	if (mtif)
	{
		ttile_t tile = TIFFComputeTile(mtif, x, y, z, 0);
		tsize_t saved = TIFFWriteEncodedTile(mtif, tile, buff, (mtileWidth*mtileHeight*mbitCount*msamplesPerPixel) / 8);
		if (saved == (mtileWidth*mtileHeight*mbitCount*msamplesPerPixel) / 8)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return false;
}
