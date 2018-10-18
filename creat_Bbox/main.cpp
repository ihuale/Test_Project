#include <iostream>
#include <string>
#include <utility>
#include <algorithm>
#include <ctime>
#include "head.h"

#include <pugixml.hpp>

void creatRandBbox(InfoList *arg_list_info, int arg_id, int arg_num)
{
	if (arg_list_info->size() > 1) {
		arg_list_info->clear();
	}
	srand((unsigned)time(0));
	for (int i = 0; i < arg_num; ++i) {
		BboxInfo tem_info;
		tem_info.pid = arg_id;
		double tem_score_int = rand() / 100.;
		tem_info.score = tem_score_int / 100;
		tem_info.class_id = rand() % 5;

		tem_info.width = std::max(60, rand() % 150);
		tem_info.height = std::max(60, rand() % 150);

		//1936 * 1216
		tem_info.ceter.x = std::max(150, std::min(1800, rand() % 1936));
		tem_info.ceter.y = std::max(150, std::min(1100, rand() % 1216));
		tem_info.print();

		//append to list
		arg_list_info->push_back(tem_info);
	}
}

bool saveBboxToXML(InfoList* arg_list, std::string arg_path, ImageID arg_id);

/*************************************************/
int main()
{
	InfoList info_list;
	int arg_id = 1;
	creatRandBbox(&info_list, arg_id, 30);
	auto tem_flag = saveBboxToXML(&info_list, "", arg_id);
	if (tem_flag) {
		std::cout << "[creat rand bboxs 0] successed!\n";
	}
	else {
		std::cout << "[creat rand bboxs 0] failed!\n";
	}

	system("pause");
	return 0;
}
/*************************************************/

bool saveBboxToXML(InfoList* arg_list, std::string arg_path, ImageID arg_id)
{
	if (arg_list->size() < 1) {
		std::cout << "[saveBboxToXML] bbox list is null!\n";
		return false;
	}
	//now,creat file
	pugi::xml_document doc;
	if (!doc.load_file((arg_path + "/result.xml").c_str(), pugi::parse_default, pugi::encoding_utf8)) {
		std::cout << "[saveBboxToXML] load successed!\n";
	}
	pugi::xml_node node_root = doc.append_child("WSScan");
	pugi::xml_node node_bboxs = node_root.append_child("Bboxs");
	//one image one bbox
	pugi::xml_node node_bbox = node_bboxs.append_child("Bbox");

	//for identification
	pugi::xml_attribute attribute_bbox = node_bbox.append_attribute("ImageID");
	attribute_bbox.set_value(arg_id);
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

	//start to write
	for (auto &tem_bbox : (*arg_list)) {
		pugi::xml_node tem_bbox_meta = node_bbox.append_child("BboxInfo");
		pugi::xml_attribute tem_image_id = tem_bbox_meta.append_attribute("ImageID");
		tem_image_id.set_value(tem_bbox.pid);

		pugi::xml_attribute tem_bbox_score = tem_bbox_meta.append_attribute("Score");
		tem_bbox_score.set_value(tem_bbox.score);

		pugi::xml_attribute tem_bbox_classid = tem_bbox_meta.append_attribute("ClassID");
		tem_bbox_classid.set_value(tem_bbox.class_id);

		pugi::xml_attribute tem_bbox_center_x = tem_bbox_meta.append_attribute("Center_X");
		tem_bbox_center_x.set_value(tem_bbox.ceter.x);
		pugi::xml_attribute tem_bbox_center_y = tem_bbox_meta.append_attribute("Center_Y");
		tem_bbox_center_y.set_value(tem_bbox.ceter.y);

		pugi::xml_attribute tem_bbox_width = tem_bbox_meta.append_attribute("Width");
		pugi::xml_attribute tem_bbox_height = tem_bbox_meta.append_attribute("Height");
		tem_bbox_width.set_value(tem_bbox.width);
		tem_bbox_height.set_value(tem_bbox.height);

	}

	//TODO
	//should write with TFRecord
	std::string tem_filename = "./result.xml";

	return doc.save_file(tem_filename.c_str());;
}