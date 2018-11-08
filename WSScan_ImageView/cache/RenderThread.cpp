#include <QGraphicsScene>
//#include "ImageFactory/Parampack.h"
#include "RenderThread.h"

RenderThread::RenderThread(QObject *parent, unsigned int nrThreads)
	: QObject(parent),
	m_abort(false),
	m_threadsWaiting(0)
{
	for (int i = 0; i < nrThreads; ++i) {
		RenderWorker* worker = new RenderWorker(this);
		//worker->start(QThread::HighPriority);
		m_workers.push_back(worker);
	}
}

RenderThread::~RenderThread()
{
	shutdown();
}

RenderJob RenderThread::getJob()
{
	m_jobListMutex.lock();
	while (m_jobList.empty() && !m_abort) {
		++m_threadsWaiting;
		m_condition.wait(&m_jobListMutex);
		--m_threadsWaiting;
	}
	if (m_abort) {
		m_jobListMutex.unlock();
		return RenderJob();
	}
	RenderJob job = m_jobList.front();
	m_jobList.pop_front();
	m_jobListMutex.unlock();
	return job;
}

//void RenderThread::addJob(const unsigned int tileSize, const float imgPosX, const float imgPosY, int bx, int by, const int level)
//{
//	RenderJob job = { tileSize, imgPosX, imgPosY, bx, by, level };
//	QMutexLocker locker(&_jobListMutex);
//	/*for (auto &it : m_jobList){
//		if (std::abs(it.imgPosX - job.imgPosX) < 0.1 && std::abs(it.imgPosY - job.imgPosY) < 0.1 && it.level == job.level){
//		return;
//		}
//		}
//		for (auto &it : _jobingList){
//		if (std::abs(it.imgPosX - job.imgPosX) < 0.1 && std::abs(it.imgPosY - job.imgPosY) < 0.1 && it.level == job.level){
//		return;
//		}
//		}
//		*/
//	m_jobList.push_front(job);
//	m_condition.wakeOne();
//}

void RenderThread::setForegroundImage(OpenSlideImageSP for_img)
{
	QMutexLocker locker(&m_jobListMutex);
	m_for_img = for_img;
	for (unsigned int i = 0; i < m_workers.size(); ++i) {
		m_workers[i]->setForegroundImage(for_img);
	}
}

void RenderThread::setForegroundOpacity(const float& opacity)
{
	m_jobListMutex.lock();
	for (unsigned int i = 0; i < m_workers.size(); ++i) {
		m_workers[i]->setForegroundOpacity(opacity);
	}
	m_jobListMutex.unlock();
}

float RenderThread::getForegroundOpacity() const
{
	if (m_workers[0]){
		return m_workers[0]->getForegroundOpacity();
	}
	else return 1.;
}

void RenderThread::clearJobs()
{
	QMutexLocker locker(&m_jobListMutex);
	if (m_jobList.size() > 0) {
		m_jobList.clear();
	}
}

void RenderThread::shutdown()
{
	m_abort = true;
	for (std::vector<RenderWorker*>::iterator it = m_workers.begin(); it != m_workers.end(); ++it) {
		(*it)->abort();
		while ((*it)->isRunning()) {
			m_condition.wakeOne();
		}
		delete (*it);
		(*it) = NULL;
	}
	m_workers.clear();
}

unsigned int RenderThread::numberOfJobs()
{
	m_jobListMutex.lock();
	unsigned int nrJobs = m_jobList.size();
	m_jobListMutex.unlock();
	return nrJobs;
}

unsigned int RenderThread::getWaitingThreads()
{
	return m_threadsWaiting;
}

std::vector<RenderWorker*> RenderThread::getWorkers()
{
	return m_workers;
}

void RenderThread::onWindowAndLevelChanged(float window, float level)
{
	m_jobListMutex.lock();
	for (unsigned int i = 0; i < m_workers.size(); ++i) {
		m_workers[i]->setWindowAndLevel(window, level);
	}
	m_jobListMutex.unlock();
}


void RenderThread::addJobBatch(std::list<RenderJob>& arg)
{
	m_jobListMutex.lock();
	//m_jobList.clear();
	//m_jobList;
	std::copy(arg.begin(), arg.end(), std::back_inserter(m_jobList));
	m_jobListMutex.unlock();
	m_condition.wakeOne();
}

void RenderThread::SwapJob(std::list<RenderJob>& arg)
{
	m_jobListMutex.lock();
	//m_jobList.clear();
	if (!m_jobList.empty()){
		m_jobList.swap(arg);
		m_jobList.clear();
	}
	m_jobListMutex.unlock();
}

bool RenderThread::AllWorkerStop()
{
	bool flag = false;
	m_jobListMutex.lock();
	 flag = m_threadsWaiting == m_workers.size();
	m_jobListMutex.unlock();
	return flag;
}

//bool RenderThread::CheckCache(RenderJob &arg)
//{
//	MPITileGraphicsItem *item = 0;
//	QString temKey = QString::number(std::floor(arg.imgPosX / tilesize)*tilesize) + "_"
//		+ QString::number(std::floor(arg.imgPosY / tilesize)*tilesize) + "_"
//		+ QString::number(arg.level);
//	bool flag = cache_->GetCache(temKey, &item);
//	return flag;
//}


