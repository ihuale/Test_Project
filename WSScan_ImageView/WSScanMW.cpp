#include "WSScanMW.h"

#include <iostream>

#include <QDir>
#include <QIcon>

WSScanMW::WSScanMW(QWidget *parent)
	: QMainWindow(parent),
	index_currretn(-1)
{
	ui.setupUi(this);
	
	setFixedSize(1200, 800);

	QIcon tem_icon_nex(":/WSScan_ImageView/Resources/next.png");
	actionChangeImage = new QAction(this);
	actionChangeImage->setIcon(tem_icon_nex);
	actionChangeImage->setShortcut(QKeySequence("N"));
	ui.mainToolBar->addAction(actionChangeImage);
	connect(actionChangeImage, &QAction::triggered, this, &WSScanMW::onActionChangeImage);

	/*list_file.push_back("E:/Pathology/WSScan/2018-11-01-211611-262/Images/2018-11-01-211611-262.svs");
	list_file.push_back("E:/Pathology/1104563893255.mrxs");*/
	//std::string filename = "E:/Pathology/WSScan/2018-11-01-211611-262/Images/2018-11-01-211611-262.svs";
	std::string filename = "E:/Pathology/1104563893255.mrxs";
	m_img = std::make_shared<OpenSlideImage>();
	m_img->initializeType(filename);

	m_viewer = new Viewer(this);
	m_viewer->initialize(m_img);

	loadDirFiles("E:/Pathology/original_data/20180111_1848000", "*.mrxs");
	onActionChangeImage();
}

WSScanMW::~WSScanMW()
{
}

bool WSScanMW::loadDirFiles(std::string arg_path, std::string arg_filter)
{
	QDir tem_dir(arg_path.c_str());
	if (!tem_dir.exists()) {
		std::cout << "[MainWindow] loadDirFiles: path is not exits: "
			<< arg_path << std::endl;
		return false;
	}
	QStringList filters;
	filters << QString(arg_filter.c_str());
	tem_dir.setFilter(QDir::Files | QDir::NoSymLinks);
	tem_dir.setNameFilters(filters);

	//get image info
	if (!list_file.empty()) {
		list_file.clear();
	}

	list_file =
		tem_dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	if (list_file.empty()) {
		std::cout << "[ImageView] loadAllImageName: no file in path:"
			<< arg_path << std::endl;
		return false;
	}

	std::sort(list_file.rbegin(), list_file.rend(),
		[&](QFileInfo &arg_1, QFileInfo &arg_2) {
		return arg_1.fileName() > arg_2.fileName(); });

	return true;
}

void WSScanMW::onActionChangeImage()
{
	std::cout << "[MainWindow] onActionmChangeImageTriggered!\n";
	if (list_file.empty()) {
		std::cout << "[MainWindow] list_file is empty!\n";
		return;
	}
	if ((index_currretn + 1) < list_file.size()) {
		++index_currretn;
	}
	else {
		index_currretn = 0;
	}
	auto tem_iter = list_file.begin() + index_currretn;

	m_img = std::make_shared<OpenSlideImage>();
	auto tem_str = tem_iter->absoluteFilePath().toStdString();
	if (m_img->initializeType(tem_str)) {
		m_viewer->close();
		m_viewer->initialize(m_img);
	}
	else {
		std::cout << "[MainWindow] open image failed!\n";
		return;
	}
}