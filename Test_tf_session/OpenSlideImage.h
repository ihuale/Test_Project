#ifndef _OPENSLIDEIMAGE_H
#define _OPENSLIDEIMAGE_H

#include <vector>
#include <string>
#include <memory>
#include <openslide/openslide.h>
#include "AIType.h"

//class Parampack;
//typedef std::shared_ptr<Parampack> ParampackSPTR;

struct RenderJob {
	float imgPosX;
	float imgPosY;
	int width, height;
	int level;
};

class OpenSlideImage {

public:

	explicit OpenSlideImage();
	~OpenSlideImage();

	bool m_isValid;

	bool initializeType(const std::string& arg_filename);

	bool readDataFromImage(RenderJob argJob,ImgDataMeta *argDes);

public:
	int m_sampleperpixel;

	std::string m_filetype;
	std::string m_errorState;

	//attribute
	int m_levels;
	int m_samplePerPixel;

	//the dimensions of image,0-x,1-y
	std::vector<std::vector<unsigned long long> > m_dims;
	std::vector<double> m_spacing;

	int m_currentLevel;
	int m_lastRenderLevel;

private:
	std::string m_current_filename;

	//ParampackSPTR _parampack;
	openslide_t *m_slide = 0;

	unsigned char m_bg_r;
	unsigned char m_bg_g;
	unsigned char m_bg_b;

};
typedef std::shared_ptr<OpenSlideImage> OpenSlideImageSP;

#endif