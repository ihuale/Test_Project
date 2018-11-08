/********************************************************************************
** Form generated from reading UI file 'WSScan_ImageView.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WSSCAN_IMAGEVIEW_H
#define UI_WSSCAN_IMAGEVIEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WSScan_ImageViewClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *WSScan_ImageViewClass)
    {
        if (WSScan_ImageViewClass->objectName().isEmpty())
            WSScan_ImageViewClass->setObjectName(QStringLiteral("WSScan_ImageViewClass"));
        WSScan_ImageViewClass->resize(600, 400);
        menuBar = new QMenuBar(WSScan_ImageViewClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        WSScan_ImageViewClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(WSScan_ImageViewClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        WSScan_ImageViewClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(WSScan_ImageViewClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        WSScan_ImageViewClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(WSScan_ImageViewClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        WSScan_ImageViewClass->setStatusBar(statusBar);

        retranslateUi(WSScan_ImageViewClass);

        QMetaObject::connectSlotsByName(WSScan_ImageViewClass);
    } // setupUi

    void retranslateUi(QMainWindow *WSScan_ImageViewClass)
    {
        WSScan_ImageViewClass->setWindowTitle(QApplication::translate("WSScan_ImageViewClass", "WSScan_ImageView", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WSScan_ImageViewClass: public Ui_WSScan_ImageViewClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WSSCAN_IMAGEVIEW_H
