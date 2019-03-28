#pragma once
#include <vector>
#include <mutex>
#include <opencv2/opencv.hpp>


template <class T>
struct QueueBuffer {
	//---------------------------------------------
	// if maxInputSize > maxCacheSize,
	// the enqueue data will overwrite the original data
	// otherwise, not
	//---------------------------------------------
	// Must be used in conjunction with semaphore, 
	// otherwise, there will be bugs(dequeue)
	//---------------------------------------------
	QueueBuffer() {
		maxInputSize = -1;//init
		//setSize(arg_maxInputSize);
	};
	~QueueBuffer() {
		clear();
	};

	void clear() {
		//who use this queue,
		//who release all queue!!!
		posLastRead = -1;
		posLastWrite = -1;
		//imgQueue.clear();
		if (maxInputSize != -1) {
			//release all memory
			//auto tem_pointer = imgQueue;
			//for (int i = 0; i < posLastWrite; ++i) {
			//	//release single img memory
			//	if ((*tem_pointer) != nullptr) {
			//		delete[](*tem_pointer);
			//		*tem_pointer = NULL;
			//	}
			//	++tem_pointer;
			//}

			//release the memory
			delete[] imgQueue;
			imgQueue = NULL;
			maxInputSize = -1;
		}

		printf("[QueueBuffer] clear all the data!\n");
	};

	void setSize(int arg_maxInputSize) {
		if (arg_maxInputSize < 1) {
			throw "[QueueBuffer] arg_maxInputSize must be greater than 1!";
			return;
		}

		/*QWriteLocker locker(&m_lock_rw);*/

		clear();

		maxInputSize = arg_maxInputSize;
		//imgQueue.setSize(maxCacheSize);
		//imgQueue = new cv::Mat*[maxInputSize];
		//imgQueue = new cv::Mat[maxInputSize];
		imgQueue = new T[maxInputSize];

		printf("[QueueBuffer] resize: maxInputSize--%d\n", maxInputSize);
	};

	bool enqueue(T arg_img) {
		std::lock_guard<std::mutex> lg(mutex);
		//auto writePos=(posLastWrite+1)%imgQueue.size();
		auto writePos = (posLastWrite + 1) % maxInputSize;//not need
		//imgQueue[writePos] = &arg_img.clone();
		imgQueue[writePos] = arg_img;
		++posLastWrite;

		return true;
	};

	T dequeue(bool *arg_flag) {
		if (posLastRead < posLastWrite) {
			posLastRead = (++posLastRead) % maxInputSize;
			*arg_flag = true;
			return imgQueue[posLastRead];
		}
		else {
			*arg_flag = false;
			return T();
		}
	};

	bool isEnqueuing() {
		//TODO
		//multi thread???
		//true: still enqueuing
		//false: enqueue alredy over

		if (posLastWrite < maxInputSize) {
			return true;
		}
		else {
			return false;
		}
	};

	bool isDequeueOver() {
		//false: still dequeuing
		//true: dequeue alredy over
		bool flag_write = (posLastWrite == maxInputSize - 1);
		bool flag_read_write = (posLastRead == posLastWrite);
		bool flag_over = flag_write && flag_read_write;
		return flag_over;
	};
private:
	//disable copy
	QueueBuffer(const QueueBuffer&);
	QueueBuffer &operator=(const QueueBuffer&);

	std::mutex mutex;

	//int maxCacheSize;
	int maxInputSize;
	volatile int posLastRead;
	volatile int posLastWrite;

public:
	//std::vector<T> imgQueue;
	//cv::Mat* imgQueue;
	T* imgQueue;
};