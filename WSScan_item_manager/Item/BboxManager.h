#pragma once

#include <iostream>
#include <string>

#include "head.h"

class ImageView;
class BboxItem;

class BboxManager
{
public:
	explicit BboxManager(ImageView* arg_view);
	~BboxManager();

	//for config
	bool initialize();

	//executed by view
	bool onVisionChanged(std::string arg_path, ImageID arg_id);

private:
	//bbox read and write
	bool readBboxFromXML(std::string arg_path, ImageID arg_id);
	bool saveBboxToXML(std::string arg_path, ImageID arg_id);

	void creatBboxItem();
	void clearBboxItem();

private:
	ImageView *m_view;

	std::string m_path;
	ImageID m_current_imageid;

	//TODO
	//should be like std::map<ImageID,InfoList>
	InfoList m_list_bbox;

	std::list<BboxItem*> m_list_current_display;
};