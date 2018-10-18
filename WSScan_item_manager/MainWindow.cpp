#include "MainWindow.h"
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	m_scene_scale_current(1.)
{
	ui.setupUi(this);
	QIcon tem_icon_1(":/WSScan_item_manager/Resources/icon_test.ico");
	setWindowIcon(tem_icon_1);
	setFixedSize(1200, 900);
	
	m_view = new ImageView(this);
	m_view->setGeometry(0, 50, 1200, 800);
	m_view->loadAllImageName();

	connect(m_view, &ImageView::showStatus, this, &MainWindow::onShowMessage);

	QIcon tem_icon_next(":/WSScan_item_manager/Resources/next.png");
	m_action_next = new QAction(this);
	m_action_next->setIcon(tem_icon_next);
	m_action_next->setShortcut(QKeySequence("N"));
	ui.mainToolBar->addAction(m_action_next);
	connect(m_action_next, &QAction::triggered, m_view, &ImageView::onDisplayNextImage);

	QIcon tem_icon_zoom_out(":/WSScan_item_manager/Resources/zoomout.png");
	m_action_zoom_out = new QAction(this);
	m_action_zoom_out->setIcon(tem_icon_zoom_out);
	m_action_zoom_out->setText(QApplication::translate("WSScan", "Zoom Out", 0));
	m_action_zoom_out->setIconText(QApplication::translate("WSScan", "Zoom Out", 0));
	m_action_zoom_out->setShortcut(QKeySequence("Z"));
	ui.mainToolBar->addAction(m_action_zoom_out);
	connect(m_action_zoom_out, &QAction::triggered, this, &MainWindow::zoomOut);

	QIcon tem_icon_zoom_in(":/WSScan_item_manager/Resources/zoomin.png");
	m_action_zoom_in = new QAction(this);
	m_action_zoom_in->setIcon(tem_icon_zoom_in);
	m_action_zoom_in->setText(QApplication::translate("WSScan", "Zoom In", 0));
	m_action_zoom_in->setIconText(QApplication::translate("WSScan", "Zoom In", 0));
	m_action_zoom_in->setShortcut(QKeySequence("X"));
	ui.mainToolBar->addAction(m_action_zoom_in);
	connect(m_action_zoom_in, &QAction::triggered, this, &MainWindow::zoomIn);

}

void MainWindow::onShowMessage(std::string arg_mes,int arg_time)
{
	if (arg_mes.empty()) {
		return;
	}
	ui.statusBar->showMessage(arg_mes.c_str(), arg_time);
}

void MainWindow::scaleScene(double arg_rate)
{
	//arg_rate: 0.9 or  1.1
	auto tem_rate = m_scene_scale_current * arg_rate;
	if (tem_rate > 1.3 || tem_rate < 0.5) {
		std::cout << "[ReviewPage] current scene scale is too small or big!\n";
		return;
	}
	m_scene_scale_current *= arg_rate;
	m_view->scale(arg_rate, arg_rate);
	std::cout << "[ReviewPage] current scene scale: " << m_scene_scale_current << std::endl;
}

void MainWindow::zoomIn()
{
	scaleScene(1.1);
}

void MainWindow::zoomOut()
{
	scaleScene(0.9);
}
