#include "RenderThread.h"

RenderThread::RenderThread(OpenSlideImageSP argImg, int nThreads):
	mfAbort(false),
	mThreadsWaitingNum(0)
{
	mImg = argImg;
	mThreads = nThreads;
}

RenderThread::~RenderThread()
{
	stop();
}

bool RenderThread::getJob(RenderJob* argJob)
{
	//TODO
	std::unique_lock<std::mutex> lk(mjobListMutex);
	if (m_jobList.empty()) {
		return false;
	}
	*argJob = m_jobList.front();
	m_jobList.pop();
	return true;
}

void RenderThread::clearJobs()
{
	std::lock_guard<std::mutex> lg(mjobListMutex);
	if (m_jobList.size() > 0) {
		std::queue<RenderJob>().swap(m_jobList);
	}
}

unsigned int RenderThread::numberOfJobs()
{
	std::lock_guard<std::mutex> lg(mjobListMutex);
	unsigned int nrJobs = m_jobList.size();

	return nrJobs;
}

void RenderThread::start()
{
	mfAbort = false;
	totalFrams = m_jobList.size();

	for (int i = 0; i < mThreads; ++i) {
		std::thread temThread(&RenderThread::run, this);
		temThread.detach();
	}
}

void RenderThread::stop()
{
	mjobListMutex.lock();
	mfAbort = true;
	mjobListMutex.unlock();
}

void RenderThread::run()
{
	if (!mImg->m_isValid) {
		printf("[RenderThread] img is NULL!\n");
		return;
	}

	//printf("[RenderWorker] one thread start!\n");

	bool running = true;
	while (running) {
		if (mfAbort && m_jobList.empty()) {
			running = false;
			break;
		}

		RenderJob _currentjob;
		auto flag = getJob(&_currentjob);
		if (!flag) {
			//get false
			running = false;
			break;
		}

		//who use imgMeta, who release
		ImgDataMeta* imgMeta = new ImgDataMeta;
		memset(imgMeta, 255, sizeof(ImgDataMeta));
		auto flag_read = mImg->readDataFromImage(_currentjob, imgMeta);
		if (flag_read) {
			//then, enqueue
			imgBuffer->enqueue(imgMeta);
		}

		mjobListMutex.lock();
		--totalFrams;
		mjobListMutex.unlock();
	}
}



