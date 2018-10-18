//#include "Processor_2.h"
//#include "utils.h"
//
//
//extern bool g_stop;
//extern QMutex g_mutex_in, g_mutex_out;
//
//int Processor_2::m_count = 0;
//
//Processor_2::Processor_2(QObject *pParent)
//	:QThread(pParent)
//{
//	m_count++;
//}
//
//
//Processor_2::~Processor_2()
//{
//	if (m_count > 0)
//		--m_count;
//}
//
//void Processor_2::setSession()
//{
//	string ROOTDIR = "H:/TestProject/TF_Test_File/";
//	string LABELS = "data/processor_2/mscoco_label_map.pbtxt";//////////////
//	string GRAPH = "data/processor_2/yjy_2.pb";//////////////
//
//	m_inputLayer = "input_1:0";//////////////
//	m_outputLayer = { "dense_2/Sigmoid:0" };//////////////бо
//
//	string graphPath = tensorflow::io::JoinPath(ROOTDIR, GRAPH);//////////////
//	LOG(INFO) << "graphPath:" << graphPath;
//
//	tensorflow::SessionOptions sessionoptions;
//	sessionoptions.config.set_log_device_placement(true);
//	sessionoptions.config.mutable_gpu_options()->set_allow_growth(true);
//	sessionoptions.config.mutable_gpu_options()->set_force_gpu_compatible(true);
//	sessionoptions.config.mutable_device_count()->insert({ "GPU",1 });
//	//creat new Session
//	m_session.reset(tensorflow::NewSession(sessionoptions));
//	if (!m_session) {
//		std::cout << "ERROR: Creating Session failed...\n" << std::endl;
//		return;
//	}
//
//	tensorflow::Status loadGraphStatus = LoadGraph(graphPath, &m_session);
//	if (!loadGraphStatus.ok()) {
//		LOG(ERROR) << "loadGraph(): ERROR" << loadGraphStatus;
//	}
//	else
//		LOG(INFO) << "loadGraph: frozen graph loaded" << std::endl;
//
//	m_shape = tensorflow::TensorShape();
//	m_shape.AddDim(1);
//	m_shape.AddDim((int64)512);
//	m_shape.AddDim((int64)512);
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
//}
//
//void Processor_2::run()
//{
//	double thresholdScore = 0.5;
//	double thresholdIOU = 0.8;
//	cv::Mat frame;
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
//		if (m_inQue->isEmpty()) {
//			g_mutex_in.unlock();
//
//		}
//		else {
//			//std::cout << "[Thread] looping inside processor worker" << std::endl;
//			frame = m_inQue->dequeue();
//			std::cout << "[Thread] Queue size is:" << m_inQue->size() << std::endl;
//			g_mutex_in.unlock();
//
//#define AI
//#ifdef AI
//
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
//			if (!runStatus.ok()) {
//				LOG(ERROR) << "Running model failed: " << runStatus;
//				break;
//				//return -1;
//			}
//
//			for (std::size_t i = 0; i < m_outputs.size(); i++) {
//				std::cout << m_outputs[i].DebugString() << std::endl;
//				//traversing the score of each image
//				auto &tem_batch_score = m_outputs[i].tensor<float, 2>();
//				for (int index_batch = 0; index_batch < m_outputs[i].dim_size(0); ++index_batch) {
//					double temMaxScore = tem_batch_score(index_batch, 0);
//					unsigned int temClassIndex = 0;
//					for (std::size_t j = 1; j < m_outputs[i].dim_size(1); ++j) {
//						//std::cout << temOutput(j);
//						//TODO
//						//j is class id
//						if (temMaxScore < tem_batch_score(index_batch, j)) {
//							temMaxScore = tem_batch_score(index_batch, j);
//							temClassIndex = j;//this is the class id of max score
//						}
//					}
//					std::cout << "[Thread] Score: " << temMaxScore << "   Class id: " << temClassIndex << std::endl;
//					g_mutex_out.lock();
//					m_outQue->enqueue(frame.clone());
//					g_mutex_out.unlock();
//				}
//			}
//#endif
//		}
//
//	}
//}
