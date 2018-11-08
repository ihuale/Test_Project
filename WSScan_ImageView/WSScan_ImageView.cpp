#include "WSScan_ImageView.h"

WSScan_ImageView::WSScan_ImageView(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	
	setFixedSize(1200, 800);

	//std::string filename = "E:/Pathology/WSScan/2018-11-01-211611-262/Images/2018-11-01-211611-262.svs";
	std::string filename = "E:/Pathology/1104563893255.mrxs";
	m_img = std::make_shared<OpenSlideImage>();
	m_img->initializeType(filename);

	m_viewer = new Viewer(this);
	m_viewer->initialize(m_img);
}
