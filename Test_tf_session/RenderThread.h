#pragma once

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "AIType.h"
#include "OpenSlideImage.h"
#include "QueueBuffer.hpp"


class RenderThread
{
public:
	RenderThread(OpenSlideImageSP argImg, int nThreads);
	~RenderThread();

	bool getJob(RenderJob* argJob);
	void clearJobs();

	void start();
	void stop();

	unsigned int numberOfJobs();

public:
	std::queue<RenderJob> m_jobList;
	QueueBuffer<ImgDataMeta*>* imgBuffer;

	volatile int totalFrams;

private:
	int mThreads;
	volatile unsigned int mThreadsWaitingNum;
	volatile bool mfAbort;

	std::mutex mjobListMutex;
	std::condition_variable mcondition;

	OpenSlideImageSP mImg;
	
	void run();
};
