#ifndef _OPENSLIDEIMAGE_H
#define _OPENSLIDEIMAGE_H

#include <string>
#include <memory>
#include <QImage>

#include "openslide/openslide.h"

//class Parampack;
//typedef std::shared_ptr<Parampack> ParampackSPTR;

struct RenderJob {
	float imgPosX;
	float imgPosY;
	int bx, by;
	int m_level;
};

class OpenSlideImage {

public:

	explicit OpenSlideImage();
	~OpenSlideImage();

	bool m_isValid;

	bool initializeType(const std::string& arg_filename);
	double getMinValue(int channel = -1) { return 0.; }
	double getMaxValue(int channel = -1) { return 255.; }

	//void setParampack(ParampackSPTR arg){ _parampack = arg; };

	std::string getProperty(const std::string& propertyName);
	std::string getOpenSlideErrorState();
	int getBestLevelForDownSample(const double& downsample) const;
	int getSamplesPerPixel(){ return m_sampleperpixel; };
	double getLevelDownsample(const unsigned int& m_level) const;
	const std::vector<double> getLevelScaleRate() const;

	void readDataFromImage(const float &startX, 
		const float &startY,
		const unsigned int &width,
		const unsigned int &height,
		const int &m_level,
		QImage &arg);

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