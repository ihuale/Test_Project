#include "CaptureThread.h"


CaptureThread::CaptureThread(QQueue<ImageMeta>* arg_inque,
	QMutex * arg_mutex,
	QObject * parent):
	m_inQue(arg_inque),
	m_mutex(arg_mutex),
	m_piece_x(50),
	m_piece_y(50),
	m_batch_size(8),
	m_image_width(512),
	m_image_height(512),
	m_image_channels(3)
{
}

CaptureThread::~CaptureThread()
{
	if (m_mutex) {
		m_mutex->unlock();
	}
	this->requestInterruption();
	this->wait();
	this->quit();
}

void CaptureThread::run()
{
	cv::Mat frame;
	Tensor tensor;
	std::vector<Tensor> outputs;

	// FPS count
	int nFrames = 32;
	int iFrame = 0;
	double fps = 0.;
	time_t start, end;
	//time(&start);
	start = clock();

	cv::VideoCapture cap(0);

	int max_num_frame = m_piece_x * m_piece_y;
	sleep(5);
	frame = cv::imread("./TF/1826.png");

	while (1 && !isInterruptionRequested()) {
	//while (cap.isOpened()) {
		//cap >> frame;
		//for normlize
		/*double tem_mat_min = 0., tem_mat_max = 0.;
		cv::minMaxIdx(frame, &tem_mat_min, &tem_mat_max);
		auto tem_mat_mid = (tem_mat_max - tem_mat_max) / 2.;
		frame = (frame - tem_mat_min) / (tem_mat_max - tem_mat_min);*/
		cv::imshow("in", frame);

		cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

		//mutex_local_main.lock();
		if (nFrames % (iFrame + 1) == 0) {
			end = clock();
			fps = nFrames / difftime(end, start) * 1000.;
			start = clock();
		}

		cv::Mat tem_frame(m_image_width, m_image_height, CV_32FC3);
		cv::resize(frame, tem_frame, cv::Size(m_image_width, m_image_height));

		m_mutex->lock();
		//inQue.enqueue(frame.clone());
		//attention for ImageID!!!
		m_inQue->enqueue(std::make_pair(tem_frame.clone(), iFrame % (max_num_frame)));
		std::cout << "frame # " << iFrame << "   Queue size:" << m_inQue->size() << std::endl;
		m_mutex->unlock();


		cv::putText(tem_frame,
			std::to_string(fps).substr(0, 5),
			cv::Point(0, tem_frame.rows),
			cv::FONT_HERSHEY_SIMPLEX,
			0.7,
			cv::Scalar(255, 0, 0));

		cv::cvtColor(tem_frame, tem_frame, cv::COLOR_BGR2RGB);
		cv::imshow("stream", tem_frame);
		cv::waitKey(5);

		iFrame++;
	}

	cv::destroyAllWindows();
}

void CaptureThread::setPiece(int arg_x, int arg_y)
{
	if (!m_mutex) {
		m_mutex = new QMutex;
	}
	m_mutex->lock();
	m_piece_x = arg_x;
	m_piece_y = arg_y;
	m_mutex->unlock();
}

void CaptureThread::setBatchInfo(
	int arg_batch_size, 
	int arg_width, 
	int arg_height, 
	int arg_channels)
{
	if (!m_mutex) {
		m_mutex = new QMutex;
	}
	m_mutex->lock();

	m_batch_size = arg_batch_size;
	m_image_width = arg_width;
	m_image_height = arg_height;
	m_image_channels = arg_channels;

	m_mutex->unlock();
}
