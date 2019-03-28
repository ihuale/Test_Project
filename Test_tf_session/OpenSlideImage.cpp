#include "OpenSlideImage.h"
#include <memory>
#include <iostream>
#include <sstream>

OpenSlideImage::OpenSlideImage():
	m_bg_r(255),
	m_bg_g(255),
	m_bg_b(255),
	m_isValid(false),
	m_samplePerPixel(3),
	m_filetype(""),
	m_errorState(""),
	m_levels(0),
	m_currentLevel(0),
	m_lastRenderLevel(0)
{
	//_parampack = std::make_shared<Parampack>();
	m_slide = 0;
}

OpenSlideImage::~OpenSlideImage()
{
	if (m_slide) {
		openslide_close(m_slide);
		m_slide = 0;
	} //need to rewrite
}

bool OpenSlideImage::initializeType(const std::string& arg_filename) {
	/*cleanup();*/

	if (!openslide_detect_vendor(arg_filename.c_str())) {
		m_isValid = false;
		return m_isValid;
	}

	m_slide = openslide_open(arg_filename.c_str());
	if (const char* str_error = openslide_get_error(m_slide)) {
		m_errorState = str_error;
		m_isValid = false;
		//throw str_error;//for test
		return m_isValid;
	}

	//here str_error is NULL or -1
	m_errorState = "";
	std::cout << "[OpenslideImage] openslide_get_error ok!\n";

	if (!m_errorState.empty()) {
		m_isValid = false;
		return m_isValid;
	}
	m_current_filename = arg_filename;

	m_levels = openslide_get_level_count(m_slide);
	m_samplePerPixel = 3;
	/*_parampack->_colorType = MPI::RGB;*/
	m_dims.clear();//for reset image
	m_spacing.clear();
	for (int i = 0; i < m_levels; ++i) {
		int64_t x, y;
		openslide_get_level_dimensions(m_slide, i, &x, &y);
		std::vector<unsigned long long> tmp;
		tmp.push_back(x);
		tmp.push_back(y);
		m_dims.push_back(tmp);
	}
	std::stringstream ssm;
	if (openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_MPP_X)) {
		ssm << openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_MPP_X);
		float tmp;
		ssm >> tmp;
		m_spacing.push_back(tmp);
		ssm.clear();
	}
	if (openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_MPP_Y)) {
		ssm << openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_MPP_Y);
		float tmp;
		ssm >> tmp;
		m_spacing.push_back(tmp);
		ssm.clear();
	}
	m_filetype = openslide_get_property_value(m_slide, OPENSLIDE_PROPERTY_NAME_VENDOR);

	// Get background color if present
	const char* bg_color_hex = openslide_get_property_value(m_slide, "openslide.background-color");
	if (bg_color_hex) {
		volatile unsigned int bg_color = std::stoi(bg_color_hex, 0, 16);
		m_bg_r = ((bg_color >> 16) & 0xff);
		m_bg_g = ((bg_color >> 8) & 0xff);
		m_bg_b = (bg_color & 0xff);
		printf("[OpenSlideImage] bg_r & bg_g & bg_b: %d  %d  %d\n", m_bg_r, m_bg_g, m_bg_b);
	}
	m_isValid = true;

	return m_isValid;
}

bool OpenSlideImage::readDataFromImage(RenderJob argJob, ImgDataMeta *argDes)
{
	if (!openslide_detect_vendor(m_current_filename.c_str())) {
		return false;
	}
	unsigned int *data = new unsigned int[argJob.width * argJob.height];
	openslide_read_region(m_slide, data, argJob.imgPosX, argJob.imgPosY, argJob.level, argJob.width, argJob.height);

	std::string _errorState;
	if (const char* error = openslide_get_error(m_slide)) {
		_errorState = error;
		printf("[OpenSlideImage] openslide_get_error: %s\n", _errorState.c_str());
		return false;
	}

	//int: 4byte = 4*8 bit
	//char && unsisgned char: 1byte = 1*8 bit
	unsigned char* rgb = new unsigned char[argJob.width * argJob.height * 3];
	unsigned char* bgra = (unsigned char*)data;
	for (unsigned long long i = 0, j = 0; i < argJob.width * argJob.height * 4; i += 4, j += 3) {
		if (bgra[i + 3] == 255) {
			rgb[j] = bgra[i + 2];
			rgb[j + 1] = bgra[i + 1];
			rgb[j + 2] = bgra[i];
		}
		else if (bgra[i + 3] == 0) {
			rgb[j] = 255;
			rgb[j + 1] = 255;
			rgb[j + 2] = 255;
		}
		else {
			//calculate rgb value
			rgb[j] = (255. * bgra[i + 2]) / bgra[i + 3];
			rgb[j + 1] = (255. * bgra[i + 1]) / bgra[i + 3];
			rgb[j + 2] = (255. * bgra[i]) / bgra[i + 3];
		}
	}
	argDes->data = rgb;
	argDes->posX = argJob.imgPosX;
	argDes->posY = argJob.imgPosY;
	argDes->width = argJob.width;
	argDes->height = argJob.height;
	argDes->level = argJob.level;

	//who get rgb, who release it
	delete[] data;
	data = NULL;

	return true;
}

