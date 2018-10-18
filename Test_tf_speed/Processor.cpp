//#include "Processor.h"
//#include "utils.h"
//
//extern bool g_stop;
//extern QMutex g_mutex_in, g_mutex_out;
//
//int Processor::m_count = 0;
//
//Processor::Processor(QObject *pParent)
//	:QThread(pParent)
//{
//	m_batch_size = 32;
//	m_count++;
//}
//
//Processor::~Processor()
//{
//	if (m_count > 0)
//		--m_count;
//}
//
//void Processor::setSession()
//{
//	string ROOTDIR = "H:/TestProject/TF_Test_File/";
//	string LABELS = "data/ssd_mobilenet_v1_coco_11_06_2017/mscoco_label_map.pbtxt";
//	string GRAPH = "data/ssd_mobilenet_v1_coco_11_06_2017/frozen_inference_graph.pb";
//
//	m_inputLayer = "image_tensor:0";
//	m_outputLayer = { "detection_boxes:0","detection_scores:0","detection_classes:0","num_detections:0" };
//
//	//std::unique_ptr<tensorflow::Session> session;
//	string graphPath = tensorflow::io::JoinPath(ROOTDIR, GRAPH);
//
//	LOG(INFO) << "graphPath:" << graphPath;
//	
//
//	tensorflow::SessionOptions sessionoptions;
//	sessionoptions.config.mutable_gpu_options()->set_allow_growth(true);
//	sessionoptions.config.mutable_gpu_options()->set_force_gpu_compatible(true);
//	sessionoptions.config.mutable_device_count()->insert({ "GPU",1 });
//	//creat new Session
//	m_session.reset(tensorflow::NewSession(sessionoptions));
//	if (!m_session) {
//		std::cout << "ERROR: Creating Session failed...\n"<< std::endl;
//		return;
//	}
//
//	Status loadGraphStatus = LoadGraph(graphPath, &m_session);
//
//	if (!loadGraphStatus.ok()) {
//		LOG(ERROR) << "loadGraph(): ERROR" << loadGraphStatus;
//	}
//	else
//		LOG(INFO) << "loadGraph: frozen graph loaded" << std::endl;
//
//	m_shape = tensorflow::TensorShape();
//	m_shape.AddDim(1);
//	m_shape.AddDim((int64)480);
//	m_shape.AddDim((int64)640);
//	m_shape.AddDim(3);
//
//	m_tensor = Tensor(tensorflow::DT_FLOAT, m_shape);
//
//	m_labelsMap = std::map<int, std::string>();
//	Status readLabelsMapStatus = readLabelsMapFile(tensorflow::io::JoinPath(ROOTDIR, LABELS), m_labelsMap);
//	if (!readLabelsMapStatus.ok()) {
//		LOG(ERROR) << "readLabelsMapFile(): ERROR" 
//			<< loadGraphStatus;
//	}
//	else
//		LOG(INFO) << "readLabelsMapFile(): labels map loaded with " 
//		<< m_labelsMap.size() 
//		<< " label(s)" 
//		<< std::endl;
//
//	std::cout << "[Thread] num: " << m_count << std::endl;
//
//}
//
//void Processor::run()
//{
//
//	double thresholdScore = 0.5;
//	double thresholdIOU = 0.8;
//	cv::Mat frame;
//	std::vector<cv::Mat> list_frame;
//
//	std::cout << "[Thread] waiting for processor worker" << std::endl;
//
//	while (!g_stop) {
//		//std::cout << "[Thread] looping inside processor worker" << std::endl;
//		/*g_mutex_in.lock();
//		std::cout << "[Thread] Queue size is:" << m_inQue->size() << std::endl;
//		g_mutex_in.unlock();*/
//
//		g_mutex_in.lock();
//		if (m_inQue->isEmpty() || m_inQue->size() < m_batch_size) {
//			g_mutex_in.unlock();
//
//		}
//		else {
//			frame = m_inQue->dequeue();
//			std::cout << "[Thread] Queue size is:" << m_inQue->size() << std::endl;
//			g_mutex_in.unlock();
//
//#define AI
//#ifdef AI
//			//translate batch image to one tensor
//			m_tensor = Tensor(tensorflow::DT_FLOAT, m_shape);
//			Status readTensorStatus = readTensorFromMat(frame, m_tensor);
//
//			if (!readTensorStatus.ok()) {
//				LOG(ERROR) << "Mat->Tensor conversion failed: " << readTensorStatus;
//			}
//			/*else
//			LOG(INFO) << "Mat->Tensor conversion suceed: " << readTensorStatus << std::endl;*/
//
//			m_outputs.clear();
//
//
//			Status runStatus = m_session->Run({ { m_inputLayer,m_tensor } },
//				m_outputLayer,
//				{},
//				&m_outputs);
//			//Status runStatus = m_session->Run({ { m_inputLayer_2,m_tensor_2 } }, m_outputLayer_2, {}, &m_outputs_2);
//			if (!runStatus.ok()) {
//				LOG(ERROR) << "Running model failed: " << runStatus;
//				break;
//				//return -1;
//			}
//			/*else
//			LOG(INFO) << "Running model succeed: " << runStatus << std::endl;*/
//
//
//			tensorflow::TTypes<float>::Flat scores = m_outputs[1].flat<float>();
//			tensorflow::TTypes<float>::Flat classes = m_outputs[2].flat<float>();
//			tensorflow::TTypes<float>::Flat numDetections = m_outputs[3].flat<float>();
//			tensorflow::TTypes<float, 3>::Tensor boxes = m_outputs[0].flat_outer_dims < float, 3>();
//
//			std::vector<size_t> goodIdxs = filterBoxes(scores, boxes, thresholdIOU, thresholdScore);
//			for (size_t i = 0; i < goodIdxs.size(); i++)
//				LOG(INFO) << "scores:" << scores(goodIdxs.at(i)) << ",class:" << m_labelsMap[classes(goodIdxs.at(i))]
//				<< " (" << classes(goodIdxs.at(i)) << "), box:" << "," << boxes(0, goodIdxs.at(i), 2) << ","
//				<< boxes(0, goodIdxs.at(i), 3);
//			cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
//			drawBoundingBoxesOnImage(frame, scores, classes, boxes, m_labelsMap, goodIdxs);
//#endif
//
//			g_mutex_out.lock();
//			m_outQue->enqueue(frame.clone());
//			g_mutex_out.unlock();
//
//		}
//
//	}
//
//}