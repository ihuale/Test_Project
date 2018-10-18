#include "test_link_two_pb.h"

#include <fstream>
#include <ctime>

#include <QDir>
#include <QList>
#include <QMutex>
#include <QQueue>
#include <QString>
#include <QFileInfo>
#include <QStringList>

#include <opencv2/core/operations.hpp>

#include "TF/ImagePreProcess.h"
#include "TF/ImageReadThread.h"

#include <pugixml.hpp>

void test_link_two_pb()
{
	/***************************************************************************************/
	tensorflow::SessionOptions *sessionoptions = new tensorflow::SessionOptions;
	sessionoptions->config.mutable_gpu_options()->set_allow_growth(true);
	sessionoptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
	sessionoptions->config.mutable_device_count()->insert({ "GPU",1 });

	std::string graph_part_1 = "/w_R2_2_model_part1.pb";
	std::string graph_part_2 = "/w_R2_2_model_part2.pb";
	//std::string tem_path_current = QDir::currentPath().toStdString();
	std::string tem_path_current = "H:/file_tensorflow/file_pb/eight_class_segment";
	//merge two path to one???
	std::string graphPath_1 = tensorflow::io::JoinPath(tem_path_current, graph_part_1);
	std::string graphPath_2 = tensorflow::io::JoinPath(tem_path_current, graph_part_2);

	LOG(INFO) << "graphPath_1:" << graphPath_1;
	LOG(INFO) << "graphPath_2:" << graphPath_2;
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::Session *session_1 = tensorflow::NewSession(*sessionoptions);
	tensorflow::Session *session_2 = tensorflow::NewSession(*sessionoptions);

	if (!session_1 || !session_2) {
		LOG(ERROR) << "[TFWorker] Creating Session failed...\n";
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::GraphDef graph_def_1, graph_def_2;
	tensorflow::Status load_graph_status_1 =
		ReadBinaryProto(tensorflow::Env::Default(),
			graphPath_1,
			&graph_def_1);
	if (!load_graph_status_1.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" + graphPath_1 + "'";
		return;
	}

	tensorflow::Status load_graph_status_2 =
		ReadBinaryProto(tensorflow::Env::Default(),
			graphPath_2,
			&graph_def_2);
	
	if (!load_graph_status_2.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" + graphPath_2 + "'";
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::Status session_create_status_1 = session_1->Create(graph_def_1);
	if (!session_create_status_1.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" << session_create_status_1;
		return;
	}
	tensorflow::Status session_create_status_2 = session_2->Create(graph_def_2);
	if (!session_create_status_2.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" << session_create_status_2;
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	//load file from disk
	QQueue<ImageCvMeta> queue_image_read;
	QMutex mutex;
	ImageReadThread thread_read(&queue_image_read, &mutex);
	thread_read.start();
	/***************************************************************************************/

	while (1) {
		/***************************************************************************************/
		mutex.lock();
		if (queue_image_read.size() < 1) {
			printf("[MainThread] waitting for input......\n");
			mutex.unlock();
			_sleep(1000);
			continue;
		}
		//start dequeue
		int tem_size = std::min(8, (int)queue_image_read.size());
		std::vector<ImageCvMeta> list_frame_mat;
		for (int i = 0; i < tem_size; ++i) {
			//Warning
			//The data in the queue has been preprocessed, 
			//so there is no need to do it again.
			list_frame_mat.push_back(queue_image_read.dequeue());
		}
		printf("[MainThread] dequeue one batch,batch size:%d\n", tem_size);
		printf("[MainThread] Queue size is: %d\n", queue_image_read.size());
		mutex.unlock();
		/***************************************************************************************/

		/***************************************************************************************/
		auto shape_input = tensorflow::TensorShape();
		shape_input.AddDim(list_frame_mat.size());//m_batch_size
		shape_input.AddDim((int64)(960));//m_image_width
		shape_input.AddDim((int64)(960));//m_image_height
		shape_input.AddDim(3);//m_image_channels
		auto tensor_input = tensorflow::Tensor(tensorflow::DT_FLOAT, shape_input);

		//std::vector<ImageCvMeta> tem_list_normalized;//no need
		std::vector<ImageCvMeta> list_resample;
		auto tem_flag_resample = ImagePreProcess::reSample(&list_frame_mat, &list_resample, 0.293, 0.243);
		tensorflow::Status readTensorStatus = ImagePreProcess::cvtMat2Tensor(&list_resample, &tensor_input);

		mutex.lock();
		std::cout << "[TFWorker] tensor_input.DebugString():" << tensor_input.DebugString() << std::endl;
		mutex.unlock();

		if (!readTensorStatus.ok()) {
			LOG(ERROR) << "[TFWorker] Mat->Tensor conversion failed: " << readTensorStatus;
			continue;
		}
		/***************************************************************************************/

		/***************************************************************************************/
		std::vector<Tensor> result_outputs_1;
		std::vector<Tensor> result_outputs_2;
		std::string ops_input_1 = "input_1:0";
		std::vector<std::string> ops_output_1 = { "activation_101/Relu:0" };
		std::string ops_input_2 = "input_2:0";
		std::vector<std::string> ops_output_2 = { "conv2d_1/truediv:0" };

		Status runStatus_1 = session_1->Run({ { ops_input_1,tensor_input } },
			ops_output_1,
			{},
			&result_outputs_1);
		if (!runStatus_1.ok()) {
			LOG(ERROR) << "[TFWorker] Running model_1 failed: " << runStatus_1;
			//system("pause");
			return;
		}
		mutex.lock();
		printf("[MainThread] session_1 run over..........\n");
		std::cout << "[MainThread]session_1 result.DebugString(): " 
			<< result_outputs_1[0].DebugString() << std::endl;
		mutex.unlock();

		//TODO
		//perform pre-processing again before performing the second calculation

		Status runStatus_2 = session_2->Run({ { ops_input_2,result_outputs_1[0] } },
			ops_output_2,
			{},
			&result_outputs_2);
		if (!runStatus_2.ok()) {
			LOG(ERROR) << "[TFWorker] Running model_2 failed: " << runStatus_2;
			return;
		}
		mutex.lock();
		printf("[MainThread] session_2 run over..........\n");
		std::cout << "[MainThread]session_2 result.DebugString(): "
			<< result_outputs_2[0].DebugString() << std::endl;
		mutex.unlock();
		/***************************************************************************************/
	}
}

void test_eight_classifier_pb()
{
	tensorflow::SessionOptions *sessionoptions = new tensorflow::SessionOptions;
	sessionoptions->config.mutable_gpu_options()->set_allow_growth(true);
	sessionoptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
	sessionoptions->config.mutable_device_count()->insert({ "GPU",1 });

	std::string graph_part = "/w_R5_3.pb";
	//std::string tem_path_current = QDir::currentPath().toStdString();
	std::string tem_path_current = "H:/file_tensorflow/file_pb/eight_classfier_segment";
	//merge two path to one???
	std::string graphPath = tensorflow::io::JoinPath(tem_path_current, graph_part);

	LOG(INFO) << "graphPath:" << graphPath;
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::Session *session = tensorflow::NewSession(*sessionoptions);

	if (!session) {
		LOG(ERROR) << "[TFWorker] Creating Session failed...\n";
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::GraphDef graph_def;
	tensorflow::Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(),
			graphPath,
			&graph_def);
	if (!load_graph_status.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" + graphPath + "'";
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::Status session_create_status_1 = session->Create(graph_def);
	if (!session_create_status_1.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" << session_create_status_1;
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	//load file from disk
	QQueue<ImageCvMeta> queue_image_read;
	QMutex mutex(QMutex::RecursionMode::Recursive);
	ImageReadThread thread_read(&queue_image_read, &mutex);
	thread_read.start();
	/***************************************************************************************/
	bool loop = true;

	std::vector<Tensor> result_total;
	std::vector<std::vector<ImageID> > result_total_id;

	time_t time_start = clock();
	while (loop) {
		/***************************************************************************************/
		mutex.lock();
		if (queue_image_read.size() < 1) {
			if (!ImagePreProcess::flag_queue) {
				//calc over
				loop = false;
				printf("[MainThread] calc over,exit while......\n");
				mutex.unlock();
				break;
			}
			printf("[MainThread] waitting for input......\n");
			mutex.unlock();
			_sleep(1000);
			continue;
		}
		//start dequeue
		int tem_size = std::min(8, (int)queue_image_read.size());
		std::vector<ImageCvMeta> list_frame_mat;
		for (int i = 0; i < tem_size; ++i) {
			//Warning
			//The data in the queue has been preprocessed, 
			//so there is no need to do it again.
			list_frame_mat.push_back(queue_image_read.dequeue());
		}
		printf("[MainThread] dequeue one batch,batch size:%d\n", tem_size);
		printf("[MainThread] Queue size is: %d\n", queue_image_read.size());
		mutex.unlock();
		/***************************************************************************************/

		/***************************************************************************************/
		auto shape_input = tensorflow::TensorShape();
		shape_input.AddDim(list_frame_mat.size());//m_batch_size
		shape_input.AddDim((int64)(960));//m_image_width
		shape_input.AddDim((int64)(960));//m_image_height
		shape_input.AddDim(3);//m_image_channels
		auto tensor_input = tensorflow::Tensor(tensorflow::DT_FLOAT, shape_input);

		//std::vector<ImageCvMeta> tem_list_normalized;//no need
		std::vector<ImageCvMeta> list_resample;
		auto tem_flag_resample = ImagePreProcess::reSample(&list_frame_mat, &list_resample, 0.293, 0.243);
		tensorflow::Status readTensorStatus = ImagePreProcess::cvtMat2Tensor(&list_resample, &tensor_input);

		mutex.lock();
		std::cout << "[TFWorker] tensor_input.DebugString():" << tensor_input.DebugString() << std::endl;
		mutex.unlock();

		if (!readTensorStatus.ok()) {
			LOG(ERROR) << "[TFWorker] Mat->Tensor conversion failed: " << readTensorStatus;
			continue;
		}
		/***************************************************************************************/

		/***************************************************************************************/
		std::vector<Tensor> result_outputs;
		std::string ops_input = "model_input:0";
		std::vector<std::string> ops_output = { "import_1/conv2d_1/truediv:0" };

		Status runStatus = session->Run({ { ops_input,tensor_input } },
			ops_output,
			{},
			&result_outputs);
		if (!runStatus.ok()) {
			LOG(ERROR) << "[TFWorker] Running model_1 failed: " << runStatus;
			//system("pause");
			return;
		}
		mutex.lock();
		printf("[MainThread] session run over..........\n");
		std::cout << "[MainThread]session result.DebugString(): "
			<< result_outputs[0].DebugString() << std::endl;
		/*auto res_tensor_mapped = result_outputs[0].tensor<float, 4>();
		auto tem_tem = res_tensor_mapped(0, 0, 0, 0);*/
		mutex.unlock();

		/***************************************************************************************/
		std::vector<ImageID> tem_vector;
		for (int i = 0; i < list_resample.size(); ++i) {
			//std::copy(result_outputs.begin(), result_outputs.end(), result_total.end());
			tem_vector.push_back(list_resample[i].second);
		}
		result_total_id.push_back(tem_vector);
		result_total.push_back(result_outputs[0]);
		/***************************************************************************************/
		//splitBbox(&result_total, &result_total_id);
	}
	time_t time_start_split = clock();
	std::cout << "[main] tf session run time:" << difftime(time_start_split, time_start) << std::endl;
	splitBbox(&result_total,&result_total_id);
	time_t time_end_split = clock();
	std::cout << "[main] tf session run time:" << difftime(time_end_split, time_start_split) << std::endl;
	std::cout << "[main] time:" << time_start << " ms\n" << time_start_split << " ms\n" << time_end_split << " ms\n";
}

void tenst_compare_pb()
{
	tensorflow::SessionOptions *sessionoptions = new tensorflow::SessionOptions;
	sessionoptions->config.mutable_gpu_options()->set_allow_growth(true);
	sessionoptions->config.mutable_gpu_options()->set_force_gpu_compatible(true);
	sessionoptions->config.mutable_device_count()->insert({ "GPU",1 });

	std::string graph_part = "/epoch3_3.pb";
	//std::string tem_path_current = QDir::currentPath().toStdString();
	std::string tem_path_current = "./model/";
	//merge two path to one???
	std::string graphPath = tensorflow::io::JoinPath(tem_path_current, graph_part);

	LOG(INFO) << "graphPath:" << graphPath;
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::Session *session = tensorflow::NewSession(*sessionoptions);

	if (!session) {
		LOG(ERROR) << "[TFWorker] Creating Session failed...\n";
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::GraphDef graph_def;
	tensorflow::Status load_graph_status =
		ReadBinaryProto(tensorflow::Env::Default(),
			graphPath,
			&graph_def);
	if (!load_graph_status.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" + graphPath + "'";
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	tensorflow::Status session_create_status_1 = session->Create(graph_def);
	if (!session_create_status_1.ok()) {
		LOG(ERROR) << "Failed to load compute graph at '" << session_create_status_1;
		return;
	}
	/***************************************************************************************/

	/***************************************************************************************/
	//load file from disk
	QQueue<ImageCvMeta> queue_image_read;
	QMutex mutex(QMutex::RecursionMode::Recursive);
	ImageReadThread thread_read(&queue_image_read, &mutex);
	thread_read.start();
	/***************************************************************************************/
	bool loop = true;

	std::vector<Tensor> result_total;
	std::vector<std::vector<ImageID> > result_total_id;

	time_t time_start = clock();
	while (loop) {
		/***************************************************************************************/
		mutex.lock();
		if (queue_image_read.size() < 1) {
			if (!ImagePreProcess::flag_queue) {
				//calc over
				loop = false;
				printf("[MainThread] calc over,exit while......\n");
				mutex.unlock();
				break;
			}
			printf("[MainThread] waitting for input......\n");
			mutex.unlock();
			_sleep(1000);
			continue;
		}
		//start dequeue
		int tem_size = std::min(8, (int)queue_image_read.size());
		std::vector<ImageCvMeta> list_frame_mat;
		for (int i = 0; i < tem_size; ++i) {
			//Warning
			//The data in the queue has been preprocessed, 
			//so there is no need to do it again.
			list_frame_mat.push_back(queue_image_read.dequeue());
		}
		printf("[MainThread] dequeue one batch,batch size:%d\n", tem_size);
		printf("[MainThread] Queue size is: %d\n", queue_image_read.size());
		mutex.unlock();
		/***************************************************************************************/

		/***************************************************************************************/
		auto shape_input = tensorflow::TensorShape();
		shape_input.AddDim(list_frame_mat.size());//m_batch_size
		shape_input.AddDim((int64)(512));//m_image_width
		shape_input.AddDim((int64)(512));//m_image_height
		shape_input.AddDim(3);//m_image_channels
		auto tensor_input = tensorflow::Tensor(tensorflow::DT_FLOAT, shape_input);

		//std::vector<ImageCvMeta> tem_list_normalized;//no need
		std::vector<ImageCvMeta> list_resample;
		//auto tem_flag_resample = ImagePreProcess::reSample(&list_frame_mat, &list_resample, 0.293, 0.486);
		tensorflow::Status readTensorStatus = ImagePreProcess::cvtMat2Tensor(&list_frame_mat, &tensor_input);

		mutex.lock();
		std::cout << "[TFWorker] tensor_input.DebugString():" << tensor_input.DebugString() << std::endl;
		mutex.unlock();

		if (!readTensorStatus.ok()) {
			LOG(ERROR) << "[TFWorker] Mat->Tensor conversion failed: " << readTensorStatus;
			continue;
		}
		/***************************************************************************************/

		/***************************************************************************************/
		std::vector<Tensor> result_outputs;
		std::string ops_input = "input_1:0";
		std::vector<std::string> ops_output = { "dense_2/Sigmoid:0" };

		Status runStatus = session->Run({ { ops_input,tensor_input } },
			ops_output,
			{},
			&result_outputs);
		if (!runStatus.ok()) {
			LOG(ERROR) << "[TFWorker] Running model_1 failed: " << runStatus;
			//system("pause");
			return;
		}
		mutex.lock();
		printf("[MainThread] session run over..........\n");
		std::cout << "[MainThread]session result.DebugString(): "
			<< result_outputs[0].DebugString() << std::endl;
		/*auto res_tensor_mapped = result_outputs[0].tensor<float, 4>();
		auto tem_tem = res_tensor_mapped(0, 0, 0, 0);*/
		mutex.unlock();

		/***************************************************************************************/
		std::vector<ImageID> tem_vector;
		for (int i = 0; i < list_resample.size(); ++i) {
			//std::copy(result_outputs.begin(), result_outputs.end(), result_total.end());
			tem_vector.push_back(list_resample[i].second);
		}
		result_total_id.push_back(tem_vector);
		result_total.push_back(result_outputs[0]);
		/***************************************************************************************/
		//splitBbox(&result_total, &result_total_id);
	}

	mutex.lock();
	for (auto &iter : result_total) {
		auto res_tensor_mapped = iter.tensor<float, 2>();
		for (int tem_index = 0; tem_index < iter.dim_size(0); ++tem_index) {
			std::cout << res_tensor_mapped(tem_index, 0) << std::endl;
		}
	}
	std::cout << "[MainThread] all batch over!\n";
	mutex.unlock();
	time_t time_start_split = clock();
	std::cout << "[main] tf session run time:" << difftime(time_start_split, time_start) << std::endl;
	std::cout << "[main] time:" << time_start << " ms\n" << time_start_split << " ms\n";
}

void splitBbox(std::vector<Tensor>* arg_in_list_tensor, 
	std::vector<std::vector<ImageID>>* arg_in_list_id)
{
	if (arg_in_list_tensor->size() < 1) {
		return;
	}
	if (arg_in_list_id->size() != arg_in_list_tensor->size()) {
		return;
	}

	std::vector<std::vector<BboxInfo> > list_bbox;

	int count_num_1 = 0;//for countting tensor
	for (auto &iter_tensor : (*arg_in_list_tensor)) {
		if (iter_tensor.dims() < 4) {
			continue;
		}
		std::vector<cv::Mat> tem_list_mat;
		int tem_batch_size = iter_tensor.dim_size(0);
		int tem_width = iter_tensor.dim_size(1);
		int tem_height = iter_tensor.dim_size(2);
		int tem_mask_num = iter_tensor.dim_size(3);
		//8* (120*120*8)
		auto flat = iter_tensor.shaped<float, 2>({ tem_batch_size,tem_height*tem_width*tem_mask_num });
		float* tem_flat_tensor = flat.data();

		int count_num_2 = 0;//for countting image
		for (int i = 0; i < tem_batch_size; ++i, ++count_num_2) {
			if (tem_flat_tensor == nullptr) {
				break;
			}
			auto tem_image_id = (*arg_in_list_id)[count_num_1][count_num_2];
			//TODO
			//delete []tem_eight_image???
			float* tem_eight_image = new float[tem_height*tem_width*tem_mask_num];
			//dst, src, size
			//get one col->get one image with tem_mask_num mask
			/*memcpy(tem_eight_image, tem_flat_tensor + i*(tem_height*tem_width*tem_mask_num), 
				sizeof(tem_eight_image));*/

			for (int iii = 0; iii < tem_height*tem_width*tem_mask_num; ++iii) {
				*(tem_eight_image + iii) = *(tem_flat_tensor + i*(tem_height*tem_width*tem_mask_num) + iii);
				//std::cout << "[splitBbox] " << *(tem_eight_image + iii) << std::endl;
			}
			//system("pause");
			double sum = 0.;//for test
			for (int iii = 0; iii < tem_mask_num; ++iii) {
				sum += *(tem_eight_image + iii);
			}
			std::cout << "[splitBbox] sum is:" << sum << std::endl;

			std::vector<BboxInfo> tem_image_bbox;

			//now,we get the eight mask;
			//(120*120) *8
			for (int ij = 0; ij < tem_mask_num; ++ij) {
				//set start pointer
				auto tem_start_ptr = tem_eight_image + ij;
				//apply for storage space
				float* tem_mask_image = new float[tem_height*tem_width];//one mask
				for (int j = 0; j < tem_height*tem_width; ++j) {
					//now, write the data
					*(tem_mask_image + j) = *(tem_start_ptr + j*tem_mask_num)/* * 2000 * 2*/;
				}
				//go to start
				//tem_mask_image -= tem_height*tem_width;
				//now,got an array that can be used to create cv::Mat
				cv::Mat tem_mat_mask(tem_width, tem_height, CV_32F, tem_mask_image);
				cv::Mat tem_mat_mask_uc(tem_width, tem_height, CV_8UC1);
				tem_mat_mask.convertTo(tem_mat_mask_uc, CV_8UC1);
				cv::threshold(tem_mat_mask_uc, tem_mat_mask_uc, 0, 255, CV_THRESH_OTSU);
				cv::Mat points;
				//cv::cvtColor(tem_mat_mask, tem_mat_mask, COLOR_BGR2GRAY);
				
				cv::findNonZero(tem_mat_mask_uc, points);

				//Mat edge_detect_canny;
				/*cv::Mat contours;
				std::vector<cv::Vec4i> hierarchy;
				findContours(tem_mat_mask_uc, contours, hierarchy,
					CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));*/

				//for (auto &iter_contou : contours) {
					cv::Rect min_rect = cv::boundingRect(points);
					if ((tem_mask_num - ij == 1) ||  (min_rect.width*min_rect.height == 0)) {
						continue;
					}
					BboxInfo tem_info;
					tem_info.pid = tem_image_id;
					tem_info.ceter.setX(((min_rect.br() + min_rect.tl())*0.5).x * 8);
					tem_info.ceter.setY(((min_rect.br() + min_rect.tl())*0.5).y * 8);
					tem_info.class_id = 0;
					tem_info.score = 0.5;
					tem_info.width = min_rect.width * 8;
					tem_info.height = min_rect.height * 8;
					tem_image_bbox.push_back(tem_info);
					//name:ImageID_maskID.jpg
					/*std::string tem_filename = "J:/data/test/2018-10-14-104136-048/Bbox/"
						+ std::to_string(tem_image_id)
						+ "_" + std::to_string(ij) + ".jpg";
					cv::imwrite(tem_filename, tem_mat_mask);
					std::cout << "[splitBbox] save one mask:" << tem_filename << std::endl;*/
					/*cv::imshow("tem_mat_mask", tem_mat_mask * 255);
					cv::waitKey(3);*/
				//}
				//delete[] tem_mask_image;
			}
			//delete[] tem_eight_image;
			list_bbox.push_back(tem_image_bbox);//one image one vector
		}
		++count_num_1;
		std::cout << "[splitBbox] one batch over!\n";
	}
	writeToXML(&list_bbox);
}

void writeToXML(std::vector<std::vector<BboxInfo> >* arg_in_list_bbox)
{
	if (arg_in_list_bbox->size() < 1) {
		return;
	}
	//creat xml root
	/*******************/
	//<version>
	//WSScan
	//	Bboxs
	//		Bbox ImageID
	//			BboxInfo image_id score class_id center_x center_y width height
	//		Bbox
	//	Bboxs
	//WSScan
	/*******************/
	pugi::xml_document doc;
	//warning!!!,must same as ImageReadThrea
	std::string tem_path = path_image_read;
	if (doc.load_file((tem_path + "/Images/bboxinfo.xml").c_str(), pugi::parse_default, pugi::encoding_utf8)) {
		std::cout << "[writeToXML] load successed!\n";
	}
	pugi::xml_node node_root = doc.append_child("WSScan");
	pugi::xml_node node_bboxs = node_root.append_child("Bboxs");


	for (auto &iter : (*arg_in_list_bbox)) {
		//iter is std::vector<BboxInfo>
		//one image one bbox
		pugi::xml_node node_bbox = node_bboxs.append_child("Bbox");

		int count_num = 0;
		//start to write
		for (auto &tem_bbox : iter) {
			if (count_num == 0) {
				//for identification
				pugi::xml_attribute attribute_bbox = node_bbox.append_attribute("ImageID");
				attribute_bbox.set_value(tem_bbox.pid);
			}

			pugi::xml_node tem_bbox_meta = node_bbox.append_child("BboxInfo");
			pugi::xml_attribute tem_image_id = tem_bbox_meta.append_attribute("ImageID");
			tem_image_id.set_value(tem_bbox.pid);

			pugi::xml_attribute tem_bbox_score = tem_bbox_meta.append_attribute("Score");
			tem_bbox_score.set_value(tem_bbox.score);

			pugi::xml_attribute tem_bbox_classid = tem_bbox_meta.append_attribute("ClassID");
			tem_bbox_classid.set_value(tem_bbox.class_id);

			pugi::xml_attribute tem_bbox_center_x = tem_bbox_meta.append_attribute("Center_X");
			tem_bbox_center_x.set_value(tem_bbox.ceter.x());
			pugi::xml_attribute tem_bbox_center_y = tem_bbox_meta.append_attribute("Center_Y");
			tem_bbox_center_y.set_value(tem_bbox.ceter.y());

			pugi::xml_attribute tem_bbox_width = tem_bbox_meta.append_attribute("Width");
			pugi::xml_attribute tem_bbox_height = tem_bbox_meta.append_attribute("Height");
			tem_bbox_width.set_value(tem_bbox.width);
			tem_bbox_height.set_value(tem_bbox.height);


			++count_num;
		}
		if (iter.size() > 0) {
			std::cout << "[writeToXML] one image bboxs write over: " << iter[0].pid << std::endl;
		}
	}

	std::string tem_filename = tem_path + "/Images/bboxinfo.xml";
	doc.save_file(tem_filename.c_str());;
	std::cout << "[writeToXML] all image bboxs write over: " << tem_filename << std::endl;
}
