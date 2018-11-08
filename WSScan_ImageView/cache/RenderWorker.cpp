#include <QImage>
#include <QPixmap>
#include "RenderWorker.h"
#include "RenderThread.h"
#include "TileGraphicsItemCache.h"
//#include "ImageFactory/Parampack.h"

RenderWorker::RenderWorker(QObject *parent):
	QThread(parent),
	m_level(0),
	m_abort(false),
	m_opacity(1.0)
{
}

RenderWorker::~RenderWorker()
{
}

void RenderWorker::abort()
{
	m_abort = true;
}

void RenderWorker::setForegroundImage(OpenSlideImageSP arg)
{
	m_mutex.lock();
	m_fgImg = arg;
	m_mutex.unlock();

}

void RenderWorker::setForegroundOpacity(double arg)
{
	m_mutex.lock();
	m_opacity = arg;
	m_mutex.unlock();
}

void RenderWorker::setWindowAndLevel(float window, float level)
{
	m_mutex.lock();
	m_window = window;
	level = level;
	m_mutex.unlock();
}

void RenderWorker::run()
{
	forever{
		//first, get job
		//second, get image data of job-region
		//third, creat pixmap
		//then, emit pixmap

		if (m_abort)
		{
			return;
		}

		RenderJob _currenjob = dynamic_cast<RenderThread *>(parent())->getJob();
		if (m_abort) {
			return;
		}
		m_mutex.lock();
		
		int _samplePerPixel = 3;//need reedit!
		if (m_fgImg) {
			_samplePerPixel = m_fgImg->getSamplesPerPixel();
		}
		else { 
			m_mutex.unlock();
			return;
		}
		QImage *temImage = new QImage;
		m_fgImg->readDataFromImage(_currenjob.imgPosX, 
			_currenjob.imgPosY,
			512, 
			512,
			_currenjob.m_level,
			*temImage);//current level
		//temImage->save("D:/"+QString::number(_currenjob.bx) + QString::number(_currenjob.by) + QString::number(_currenjob.level) + ".jpg","jpg");

		QPixmap *renderPixmap = new QPixmap;
		renderPixmap->convertFromImage(*temImage);

		emit tileLoaded(renderPixmap,
			_currenjob.imgPosX,
			_currenjob.imgPosY,
			_currenjob.bx,
			_currenjob.by,
			_currenjob.m_level);

		delete temImage;
		//temImage = NULL;

		m_mutex.unlock();

	}
}
