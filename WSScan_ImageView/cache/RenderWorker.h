#pragma once
#include <memory>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QPixmap>

#include "ImageFactory/OpenSlideImage.h"

//struct RenderJob;

class MPITileGraphicsItem;

class RenderWorker : public QThread
{
	Q_OBJECT

public:
	RenderWorker(QObject *parent);
	~RenderWorker();
	void abort();

	void setForegroundImage(OpenSlideImageSP arg);
	void setForegroundOpacity(double arg);
	void setWindowAndLevel(float, float);
	double getForegroundOpacity(){ return m_opacity; };

	QPixmap readForegroundImage(OpenSlideImageSP local_for_img, const RenderJob &currentJob);
	QPixmap readBackgroundImage(OpenSlideImageSP local_for_img, const RenderJob &currentJob);

	//MPITileGraphicsItem *item_ = 0;

signals:
	void tileLoaded(QPixmap *tile, 
		float tileX, float tileY, 
		int bx, int by, 
		int tileLevel);
	//void selfFinish(RenderWorker*);

protected:
	void run();

private:
	QMutex m_mutex;
	OpenSlideImageSP m_fgImg;

	bool m_abort;
	int m_level;
	float m_window;
	double m_opacity;
};
