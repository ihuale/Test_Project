#pragma once

#include <list>

#include <QThread>
#include <QQueue>

#include <opencv2/opencv.hpp>
#include "utils.h"

class CheckOutputQue : public QThread
{
	Q_OBJECT

public:
	explicit CheckOutputQue(QQueue<ResultMeta> *que, 
		QObject *parent = nullptr);
	explicit CheckOutputQue(std::list<ResultMeta> *que,
		QObject *parent = nullptr);
	explicit CheckOutputQue(ResultMap *map,
		QObject *parent = nullptr);
	~CheckOutputQue();

	void run() override;

private:
	QQueue<ResultMeta> *m_outQue;
	std::list<ResultMeta> *m_out_list;
	ResultMap *m_map;

	bool m_flag_queue;
	bool m_flag_list;
	bool m_flag_map;
};
