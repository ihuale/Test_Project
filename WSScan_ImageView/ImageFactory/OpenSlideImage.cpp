#include <memory>
#include <QDebug>
#include <QTime>
#include <iostream>
#include <sstream>
#include "OpenSlideImage.h"
//#include "ImageFactory/MPIEnums.h"
//#include "ImageFactory/Parampack.h"

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

	if (openslide_detect_vendor(arg_filename.c_str())) {
		m_slide = openslide_open(arg_filename.c_str());
		if (const char* str_error = openslide_get_error(m_slide)) {
			m_errorState = str_error;
			m_isValid = false;
			//throw str_error;//for test
			return false;
		}
		else {
			//here str_error is NULL or -1
			m_errorState = "";
			std::cout << "[OpenslideImage] openslide_get_error ok!\n";
		}
		if (m_errorState.empty()) {
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
				qDebug() << "[OpenSlideImage] bg_r & bg_g & bg_b: " 
					<< m_bg_r << " " << m_bg_g << " " << m_bg_b << "\n";
			}
			m_isValid = true;
		}
		else {
			m_isValid = false;
		}
	}
	else {
		m_isValid = false;
	}
	return m_isValid;
}

int OpenSlideImage::getBestLevelForDownSample(const double& downsample) const
{
	if (m_isValid) {
		float previousDownsample = 1.0;
		if (downsample < 1.0) {
			return 1;
		}
		for (int i = 1; i < m_levels; ++i) {
			double currentDownSample = (double)getLevelDownsample(i);
			double previousDownSample = (double)getLevelDownsample(i - 1);
			if (downsample < currentDownSample) {
				if (std::abs(currentDownSample - downsample) > std::abs(previousDownSample - downsample)) {
					return i - 1;
				}
				else {
					return i;
				}
				return i - 1;
			}
			//else continue;
		}
		return (m_levels - 1);
	}
	else {
		return -1;
	}
}

double OpenSlideImage::getLevelDownsample(const unsigned int& m_level) const
{
	if (m_isValid && (m_level < m_levels)) {
		return static_cast<float>(m_dims[0][0]) / m_dims[m_level][0];
	}
	else {
		return -1.0;
	}
}

const std::vector<double> OpenSlideImage::getLevelScaleRate() const
{
	std::vector<double> temVec;
	if (!m_current_filename.empty()) {
		for (int index1 = 0; index1 < m_dims.size(); ++index1) {
			temVec.push_back((this->m_dims[index1][0]) / this->m_dims[m_lastRenderLevel][0]);
		}
	}
	return temVec;
}

void OpenSlideImage::readDataFromImage(const float &startX, 
	const float &startY,
	const unsigned int &width,
	const unsigned int &height,
	const int &m_level,
	QImage &arg)
{
	if (!openslide_detect_vendor(m_current_filename.c_str())) {
		return ;
	}
	unsigned int *data = new unsigned int[width * height];
	openslide_read_region(m_slide, data, startX, startY, m_level, width, height);//!!!

	std::string _errorState;
	if (const char* error = openslide_get_error(m_slide)) {
		_errorState = error;
		qDebug() << "This is openslide_get_error: " << QString::fromStdString(_errorState);
		return;
	}

	unsigned char* rgb = new unsigned char[width * height * 3];
	unsigned char* bgra = (unsigned char*)data;
	for (unsigned long long i = 0, j = 0; i < width * height * 4; i += 4, j += 3) {
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
			rgb[j] = (255. * bgra[i + 2]) / bgra[i + 3];
			rgb[j + 1] = (255. * bgra[i + 1]) / bgra[i + 3];
			rgb[j + 2] = (255. * bgra[i]) / bgra[i + 3];
		}
	}

	//arg = QImage(reinterpret_cast<unsigned char*>(rgb),
	//	width,
	//	height,
	//	width * 4,
	//	QImage::Format_ARGB32).convertToFormat(QImage::Format_RGB888);//QImage renderImage

	arg = QImage(rgb,
		width,
		height,
		width * 3,
		QImage::Format_RGB888);//QImage renderImage

	//qDebug() << "[OpenSlideImage] arg image:" << arg;
	//std::cout << "[OpenSlideImage] read image over!\n";

	delete[] data;
	//delete[] rgb;
	//delete[] bgra;
}

