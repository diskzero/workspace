/*
The MIT License (MIT)
Copyright (c) 2011 Gene Z. Ragan
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// Self
#include "mainwindow.h"

// Qt
#include <QPushButton>
#include <QTreeWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(800, 800);

    QPushButton* pushButton = new QPushButton("Click Me", this);
    pushButton->setParent(this);
    pushButton->move(10, 10);

    pushButton = new QPushButton("Click Me", this);
    pushButton->setParent(this);
    pushButton->move(10, 60);
    pushButton->setEnabled(false);

    QTreeWidget* treeWidget = new QTreeWidget(this);
    treeWidget->setColumnCount(2);
    QStringList headers;
    headers << tr("Subject") << tr("Default");
    treeWidget->setHeaderLabels(headers);
    treeWidget->move(200, 10);
    treeWidget->resize(300, 300);


    QTabBar* tabBar= new QTabBar(this);
    tabBar->addTab("One");
    tabBar->addTab("Two");
    tabBar->addTab("Three");
    tabBar->move(200, 400);
    tabBar->resize(300, 300);

}


MainWindow::~MainWindow()
{

}
