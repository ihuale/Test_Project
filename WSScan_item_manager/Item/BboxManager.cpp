#include "BboxManager.h"

#include <QDir>
#include <QPoint>
#include <QFileInfo>

#include <pugixml.hpp>

#include "ImageView.h"
#include "Item/BboxItem.h"

BboxManager::BboxManager(ImageView *arg_view):
	m_view(arg_view),
	m_current_imageid(-1)
{
}

BboxManager::~BboxManager()
{
	if (m_list_bbox.size() > 1) {
		m_list_bbox.clear();
	}
	std::cout << "[BboxManager] ~BboxManager()\n";
}

bool BboxManager::initialize()
{
	return false;
}

bool BboxManager::onVisionChanged(std::string arg_path, ImageID arg_id)
{
	if (arg_id == m_current_imageid) {
		return true;
	}
	else {
		clearBboxItem();
	}
	QDir tem_dir(arg_path.c_str());
	if (!tem_dir.exists()) {
		std::cout << "[BboxManager] arg_path does not exists: " << arg_path << std::endl;
		return false;
	}
	std::string tem_filename = arg_path + "/Images/" + std::to_string(arg_id) + ".xml";
	QFileInfo tem_info(tem_filename.c_str());
	if (!tem_dir.exists()) {
		std::cout << "[BboxManager] file does not exists: " << tem_filename << std::endl;
		return false;
	}
	m_path = arg_path;
	m_current_imageid = arg_id;
	
	auto tem_read_flag = readBboxFromXML(arg_path, arg_id);
	if (tem_read_flag) {
		creatBboxItem();
		return true;
	}
	else {
		return false;
	}
}

bool BboxManager::readBboxFromXML(std::string arg_path, ImageID arg_id)
{
	//from onVisionChanged,arg has been checked
	QDir tem_dir(arg_path.c_str());
	if (arg_path.size() < 2|| !tem_dir.exists()) {
		//error filename!!
		std::cout << "[ResultOperate] path does not exits: " << arg_path << std::endl;
		return false;
	}
	std::string tem_filename = arg_path + "/Images/bboxinfo.xml";
	QFileInfo info(tem_filename.c_str());
	if (!info.exists()) {
		std::cout << "[ResultOperate] file does not exists: " << tem_filename << std::endl;
		return false;
	}

	//start to read
	pugi::xml_document xml_doc;
	pugi::xml_parse_result tree = xml_doc.load_file(tem_filename.c_str());
	pugi::xml_node root = xml_doc.child("WSScan");
	if (root.empty()) {
		std::cout << "[BboxManager] could not find root:" << tem_filename << std::endl;
		return false;
	}
	pugi::xml_node tem_bboxs = root.child("Bboxs");

	if (m_list_bbox.size() > 1) {
		m_list_bbox.clear();
	}

	/*******************/
	//<version>
	//WSScan
	//	Bboxs
	//		Bbox ImageID
	//			BboxInfo ImageID Score ClassID Center_X Center_Y Width Height
	//		Bbox
	//		Bbox ImageID
	//			BboxInfo ImageID Score ClassID Center_X Center_Y Width Height
	//		Bbox
	//	Bboxs
	//WSScan
	/*******************/

	//TODO
	auto tem_children = tem_bboxs.children();
	auto tem_finder = std::find_if(tem_children.begin(), tem_children.end(), 
		[&](pugi::xml_node arg_node) {
		auto tem_name = arg_node.name();
		//bool flag_name = arg_node.name() == "Bbox";//ERROR when using,c not use that!!!
		bool flag_name = (strcmp(arg_node.name(), "Bbox") == 0);
		bool flag_attribute = (arg_node.attribute("ImageID").as_int() == arg_id);
		return (flag_name && flag_attribute);
	});
	//auto tem_finder = tem_bboxs.find_child("Bbox", "ImageID", std::to_string(arg_id).c_str());

	if (tem_finder == tem_children.end()) {
		printf("[BboxManager] can not find bbox in image: %d\n", arg_id);
		return false;
	}

	//traverse
	auto tem_image_bbox = tem_finder->children();
	for (pugi::xml_node_iterator iter_bboxs = tem_image_bbox.begin();
		iter_bboxs != tem_image_bbox.end(); ++iter_bboxs)
	{
		//read meta
		BboxInfo tem_info;
		tem_info.pid = iter_bboxs->attribute("ImageID").as_int();
		tem_info.score = iter_bboxs->attribute("Score").as_double();
		tem_info.class_id = iter_bboxs->attribute("ClassID").as_int();

		tem_info.width = iter_bboxs->attribute("Width").as_int();
		tem_info.height = iter_bboxs->attribute("Height").as_int();

		auto tem_x = iter_bboxs->attribute("Center_X").as_int();
		auto tem_y = iter_bboxs->attribute("Center_Y").as_int();
		tem_info.ceter = QPoint(tem_x, tem_y);

		//add to list
		m_list_bbox.push_back(tem_info);

		//read completed
	}
	std::cout << "[ResultOperate] read result over: " << tem_filename << std::endl;

	return true;
}

bool BboxManager::saveBboxToXML(std::string arg_path, ImageID arg_id)
{
	if (m_list_bbox.size() < 1) {
		std::cout << "[BboxManager] bbox list is null!\n";
		return false;
	}
	QDir tem_dir(arg_path.c_str());
	if (!tem_dir.exists()) {
		auto tem_flag_creat = tem_dir.mkpath(arg_path.c_str());
		if (!tem_flag_creat) {
			return false;
		}
	}
	//now,creat file
	pugi::xml_document doc;
	pugi::xml_node node_root = doc.append_child("WSScan");
	pugi::xml_node node_bboxs = node_root.append_child("Bboxs");
	pugi::xml_node node_bbox = node_bboxs.append_child("Bbox");
	//for identification
	pugi::xml_attribute node_bbox_imageid = node_bboxs.append_attribute("ImageID");
	node_bbox_imageid.set_value(arg_id);
	//node_bbox.set_value(std::to_string(arg_id).c_str());

	/*******************/
	//<version>
	//WSScan
	//	Bboxs
	//		Bbox ImageID
	//			BboxInfo ImageID Score ClassID Center_X Center_Y Width Height
	//		Bbox
	//	Bboxs
	//WSScan
	/*******************/

	//start to write
	for (auto &tem_bbox : m_list_bbox) {
		pugi::xml_node tem_bbox_meta = node_bbox.append_child("BboxInfo");
		pugi::xml_attribute tem_image_id = tem_bbox_meta.append_attribute("PID");
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

	}

	//TODO
	//should write with TFRecord
	std::string tem_filename = arg_path + "/bboxinfo.xml";

	return doc.save_file(tem_filename.c_str());;
}

void BboxManager::creatBboxItem()
{
	if (m_list_bbox.size() < 1) {
		return;
	}
	auto tem_scene = m_view->scene();

	clearBboxItem();

	for (auto &tem_iter : m_list_bbox) {
		BboxInfo tem_info;// = /*{ QPointF(100,100),100,100,2,0.65,3 };*/
		tem_info = tem_iter;
		BboxItem *item_bbox = new BboxItem(tem_info, "#39FF33", m_view);
		m_list_current_display.push_back(item_bbox);
		tem_scene->addItem(item_bbox);
		item_bbox->setPos(tem_info.ceter);
		tem_scene->update();
	}
}

void BboxManager::clearBboxItem()
{
	if (m_list_current_display.size() < 1)
		return;
	//TODO
	auto tem_scene_list_item = m_view->scene()->items();
	for (auto &tem_iter : tem_scene_list_item) {
		auto tem_case = dynamic_cast<BboxItem*>(tem_iter);
		if (tem_case) {
			m_view->scene()->removeItem(tem_case);
		}
	}
	m_list_current_display.clear();
	//delete m_list_current_display.front();
}
