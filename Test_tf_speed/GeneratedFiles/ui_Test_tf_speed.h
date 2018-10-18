/********************************************************************************
** Form generated from reading UI file 'Test_tf_speed.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_TF_SPEED_H
#define UI_TEST_TF_SPEED_H

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

class Ui_Test_tf_speedClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Test_tf_speedClass)
    {
        if (Test_tf_speedClass->objectName().isEmpty())
            Test_tf_speedClass->setObjectName(QStringLiteral("Test_tf_speedClass"));
        Test_tf_speedClass->resize(600, 400);
        menuBar = new QMenuBar(Test_tf_speedClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        Test_tf_speedClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(Test_tf_speedClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        Test_tf_speedClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(Test_tf_speedClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        Test_tf_speedClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(Test_tf_speedClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        Test_tf_speedClass->setStatusBar(statusBar);

        retranslateUi(Test_tf_speedClass);

        QMetaObject::connectSlotsByName(Test_tf_speedClass);
    } // setupUi

    void retranslateUi(QMainWindow *Test_tf_speedClass)
    {
        Test_tf_speedClass->setWindowTitle(QApplication::translate("Test_tf_speedClass", "Test_tf_speed", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Test_tf_speedClass: public Ui_Test_tf_speedClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_TF_SPEED_H
