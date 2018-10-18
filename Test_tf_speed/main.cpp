#include <QApplication>

#ifndef COMPILER_MSVC
#define COMPILER_MSVC
#endif //COMPILER_MSVC

#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX

#include <stdio.h>
#include <tchar.h>

#include <numeric>
#include <eigen/Dense>
#include <fstream>
#include <utility>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <regex>

#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/str_util.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

#include <opencv2/opencv.hpp>

#include <QQueue>
#include <QMutex>
#include <QVector>
#include <QThread>
#include <QRunnable>
#include <QThreadPool>
#include <QObject>

//#include "utils.h"
#include "CheckOutputQue.h"
#include "TFWorker.h"
#include "TFResultProcessor.h"
#include "TileMap.h"
#include "CaptureThread.h"



//These are all common classes 
//it's handy to reference with no namespace.
using tensorflow::Flag;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

bool g_stop = false;
QMutex g_mutex_in, g_mutex_out;


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QWidget win_widget;
	//win_widget.resize(400, 300);
	int tem_piece_x = 30, tem_piece_y = 30;
	win_widget.setFixedSize(tem_piece_x * 10 + 2, tem_piece_y * 5 + 2);
	
	TileMap m_label(&win_widget, tem_piece_x, tem_piece_y);
	//m_label.setDirection(TileMap::ColFirst);
	m_label.setFocus(Qt::FocusReason::OtherFocusReason);
	win_widget.show();

	QQueue<ImageMeta> inQue;
	//std::list<ResultMeta> out_list;
	ResultMap out_list;
	QVector<TFWorker *> workers;

	int numWorkers = 2;
	int batch_size = 32;
	int image_width = 299;
	int image_height = 299;
	int image_channels = 3;

	for (int i = 0; i != numWorkers; i++) {
		workers.append(new TFWorker(&win_widget));
		workers[i]->setGraphFile(
			"model/inceptionv4/single_graph_static.pb",
			"fifo_queue_Dequeue",
			{ "InceptionV4/Logits/Predictions" },
			batch_size, image_width, image_height, image_channels );
		workers[i]->setQueues(&inQue, &out_list);
		workers[i]->setQueueMutex(g_mutex_in, g_mutex_out);
		workers[i]->setWorkState(true);
		workers[i]->setTileMap(&m_label);
		workers[i]->start();
	}

	CheckOutputQue* checkWorker = new CheckOutputQue(&out_list);
	checkWorker->start();

	CaptureThread capturer(&inQue, &g_mutex_in);
	capturer.setPiece(tem_piece_x, tem_piece_y);
	capturer.setBatchInfo(batch_size, image_width, image_height, image_channels);
	capturer.start();

	//getchar();

	//a.quit();

	return a.exec();
}
