#pragma once

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <list>
#include <vector>
#include "cache/RenderWorker.h"
#include "ImageFactory/OpenSlideImage.h"




class RenderThread : public QObject
{
	Q_OBJECT

public:
	RenderThread(QObject *parent, unsigned int nrThreads = 1);
	~RenderThread();

	RenderJob getJob();
	//void addJob(const float imgPosX, const float imgPosY, int bx, int by, const int level);
	void addJobBatch(std::list<RenderJob>&);
	void removeJobing(const RenderJob&);

	void setForegroundImage(OpenSlideImageSP for_img);
	void setForegroundOpacity(const float& opacity = 1.0);
	float getForegroundOpacity() const;

	void clearJobs();
	void shutdown();
	unsigned int numberOfJobs();
	unsigned int getWaitingThreads();

	void SwapJob(std::list<RenderJob>&);

	std::vector<RenderWorker*> getWorkers();
	std::list<RenderJob> m_jobList;

	bool AllWorkerStop();

signals:
	void clearQGraphicsItem();

public slots:
	void onWindowAndLevelChanged(float window, float level);

	

private:
	
	bool m_abort;
	QMutex m_jobListMutex;
	//QMutex _jobingMutex;
	QWaitCondition m_condition;
	OpenSlideImageSP m_for_img;
	
	std::vector<RenderWorker*> m_workers;
	unsigned int m_threadsWaiting;
};
