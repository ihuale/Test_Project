#include "ImageReadThread.h"

#include <QDir>
#include <QList>
#include <QString>
#include <QFileInfo>
#include <QStringList>

#include "TF/ImagePreProcess.h"

ImageReadThread::ImageReadThread(QQueue<ImageCvMeta> *arg_out_list,
	QMutex* arg_mutex,
	QObject *parent):
	QThread(parent),
	m_out_queue(arg_out_list),
	m_mutex(arg_mutex),
	m_read_path(path_image_read)
{
}

ImageReadThread::~ImageReadThread()
{
	wait();
	quit();
	//destroyed();
}

void ImageReadThread::setPath(std::string arg_path)
{
}

void ImageReadThread::run()
{
	//dectec whether have image on disk
	std::vector<ImageCvMeta> list_frame_mat;
	QDir dir((m_read_path + "Images/").c_str());
	QStringList filter;
	//filter << image_filter.c_str();
	filter << "*.tif";
	dir.setNameFilters(filter);
	QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir.entryInfoList(filter));
	auto image_count = fileInfo->count();
	if (image_count < 1) {
		std::cout << "[ImageReadThread] no image on path: " << m_read_path << std::endl;
		return;
	}
	//auto tem_count = fileInfo->size() / 1.2;
	/*auto tem_count = 446;
	for (int i = 0; i < tem_count; ++i) {
		fileInfo->removeLast();
	}*/

	m_mutex->lock();
	ImagePreProcess::flag_queue = true;
	std::cout << "[ImageReadThread] start to read!\n";
	m_mutex->unlock();

	int count_index = 0;
	for(auto &iter:(*fileInfo)) {
		//std::vector<cv::Mat> tem_list;
		//auto tem_mat = cv::imreadmulti("", tem_list, 2);
		auto tem_filename = iter.absoluteFilePath().toStdString();
		auto tem_mat = cv::imread(tem_filename);
		auto teme_filename_index = iter.fileName().remove(-4, 4);
		ImageCvMeta tem_pair = std::make_pair(tem_mat.clone(), teme_filename_index.toInt());
		//ImagePreProcess::normlize(&tem_pair);

		//enqueue
		m_mutex->lock();
		m_out_queue->enqueue(tem_pair);
		++count_index;
		std::cout << "[ImageReadThread] read image: " << tem_filename << std::endl;
		m_mutex->unlock();

		tem_mat.release();
	}

	m_mutex->lock();
	ImagePreProcess::flag_queue = false;
	std::cout << "[ImageReadThread] read over!\n";
	std::cout << "[ImageReadThread] *********************************\n";
	std::cout << "[ImageReadThread] *********************************\n";
	std::cout << "[ImageReadThread] *********************************\n";
	std::cout << "[ImageReadThread] *********************************\n";
	std::cout << "[ImageReadThread] *********************************\n";
	m_mutex->unlock();
}
