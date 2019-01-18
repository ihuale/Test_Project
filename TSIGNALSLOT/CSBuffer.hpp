#pragma once
#include <vector>
#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include <QDebug>

struct CSBuffer {
// if maxInputSize > maxCacheSize,
// the enqueue data will overwrite the original data 
// otherwise, not
	CSBuffer(int arg_maxInputSize = 1000,
		int arg_maxCacheSize = 100) {
		if (arg_maxInputSize < arg_maxCacheSize) {
			qDebug() << "[CSBuffer] arg_maxInputSize must be greater than arg_maxCacheSize!";
			throw "[CSBuffer] arg_maxInputSize must be greater than arg_maxCacheSize!";
		}
		maxCacheSize = arg_maxCacheSize;
		maxInputSize = arg_maxInputSize;
		posLastRead = -1;
		posLastWrite = -1;
		imgQueue.resize(arg_maxCacheSize);
	};
	~CSBuffer() {
		clear();
	};
	void clear() {
		for (auto &iter : imgQueue) {
			iter.release();
		}
		imgQueue.clear();
	};

	void resize(int arg_maxInputSize,int arg_maxCacheSize) {
		if (arg_maxInputSize < 1 || arg_maxCacheSize < 1)
			return;
		clear();
		maxInputSize = arg_maxInputSize;
		maxCacheSize = arg_maxCacheSize;
		posLastRead = -1;
		posLastWrite = -1;
		imgQueue.resize(maxCacheSize);
	};
	bool enqueue(cv::Mat arg_img) {
		//posLastWrite grow from 0
		if (posLastWrite >= maxInputSize) {
			//TODO
			//print error
			qDebug() << "[CSBuffer] alredy write size is: " << posLastWrite
				<< "  maxInputSzie: " << maxInputSize;
			return false;
		}
		//auto writePos=(posLastWrite+1)%imgQueue.size();
		auto writePos = (posLastWrite + 1) % maxCacheSize;
		imgQueue[writePos] = arg_img;
		++posLastWrite;

		return true;
	};
	cv::Mat* dequeue() {
		//auto readPos=(posLastRead+1)%imgQueue.size();
		posLastRead = (++posLastRead) % maxCacheSize;
		return &imgQueue[posLastRead];
	};

public:
	std::vector<cv::Mat> imgQueue;

private:
	CSBuffer(const CSBuffer&);
	CSBuffer &operator=(const CSBuffer&);

	int maxCacheSize;
	int maxInputSize;
	int posLastRead;
	int posLastWrite;
};