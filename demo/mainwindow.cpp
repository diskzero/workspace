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

// Qt
#include <QtGui>
#include <QtWidgets>

// Local
#include "mainwindow.h"
#include "../Workspace.h"
#include "../WorkspaceArea.h"
#include "../WorkspaceLayout.h"
#include "../WorkspacePanel.h"
#include "../WorkspacePanelGroup.h"

MainWindow::MainWindow()
{
    setObjectName("MainWindow");
    init();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::newFile()
{
}

WorkspacePanel*
MainWindow::getNewPanel(const QString& title)
{
    WorkspacePanel* panel = new WorkspacePanel(title, centralWidget());
    QTextEdit* textEdit = new QTextEdit(panel);
    textEdit->setText(title);
    panel->setWidget(textEdit);
    return panel;
}

void MainWindow::newPanel()
{
    static int nPanels = 0;
    ++nPanels;
    QString title(tr("Panel %1").arg(nPanels));
    workspaceArea->addPanel(getNewPanel(title), Qt::Horizontal);
}

void MainWindow::open()
{
}

bool MainWindow::save()
{
    return false;
}

bool MainWindow::saveAs()
{
    return false;
}

void
MainWindow::about()
{
   QMessageBox::about(this, tr("About Workspace Demo"),
            tr("The example demonstrates the Workspace."));
}

void
MainWindow::init()
{
	setWindowTitle("Workspace Demo");
	
    setAttribute(Qt::WA_DeleteOnClose);

    isUntitled = true;

    workspaceArea = new WorkspaceArea();
    setCentralWidget(workspaceArea);

    createActions();
    createMenus();
    createStatusBar();

    readSettings();

    setUnifiedTitleAndToolBarOnMac(true);

    workspaceArea->beginDeferLayout();
    
#if 1
    // Get the user state files
    QFile stateFile("./workspace.conf");
    if (!stateFile.exists()) {
        // Try to use the defaults
        stateFile.setFileName(":defaultWorkspace");
    }
        
	if (stateFile.exists()) {

        stateFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QXmlStreamReader theStream(&stateFile);
	
        // Try to restore state
        WorkspaceArea::SavedLayout theLayout;
        workspaceArea->restoreState(theStream, theLayout);

        WorkspaceArea::SavedGroups& theGroups = theLayout.second;
        
        // Create the panel groups and panels
        QMapIterator<QString, WorkspaceArea::SavedGroup> mapIter(theGroups);
        while (mapIter.hasNext()) {
            mapIter.next();

            // Get the data for the panel group
            const WorkspaceArea::SavedGroup& layoutData = mapIter.value();

            // Create the panel group
            WorkspacePanel* groupPanel = NULL;
            
            // Add the panels to the group
            Q_FOREACH(QString panelName, layoutData.panels) {
                if (groupPanel == NULL) {
                    groupPanel = getNewPanel(panelName);
                    workspaceArea->addPanel(groupPanel, Qt::Horizontal);
                } else {
                    WorkspacePanel* thePanel = getNewPanel(panelName);
                    workspaceArea->addToPanelGroup(groupPanel, thePanel);
                }
            }

            // Set the active panel of the group
            WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(workspaceArea->layout());
            WorkspacePanelGroup* panelGroup = workspaceLayout->findPanelGroup(groupPanel);
            panelGroup->setCurrentIndex(layoutData.active);

            // Set the group grid position
            panelGroup->setConstraints(layoutData.gridx, 
                                       layoutData.gridy, 
                                       layoutData.gridwidth, 
                                       layoutData.gridheight);
		}	

        if (theLayout.first != workspace::Workspace::DefaultLayoutName)
            workspaceArea->setLayoutName(theLayout.first);
        else
            workspaceArea->setLayoutName("Unnamed Layout");

        workspaceArea->restoreComplete();


        stateFile.close();    

    } else
#endif
{
#if 0
        const bool BEFORE = true;
        const bool AFTER = !BEFORE;

        // Initialize state
        WorkspacePanel* panelOne = getNewPanel("One");
        WorkspacePanel* panelTwo = getNewPanel("Two");
        WorkspacePanel* panelThree = getNewPanel("Three");
        WorkspacePanel* panelFour = getNewPanel("Four");
        WorkspacePanel* panelFive = getNewPanel("Five");
        WorkspacePanel* panelSix = getNewPanel("Six");
        WorkspacePanel* panelSeven = getNewPanel("Seven");
        WorkspacePanel* panelEight = getNewPanel("Eight");

        workspaceArea->addPanel(panelOne, Qt::Horizontal);
        workspaceArea->insertPanel(panelOne, panelTwo    , Qt::Horizontal, AFTER);

        workspaceArea->insertPanel(panelOne, panelThree,  Qt::Vertical  , AFTER);
        workspaceArea->insertPanel(panelThree, panelFour  , Qt::Horizontal, AFTER);

        workspaceArea->insertPanel(panelOne, panelFive ,   Qt::Horizontal, AFTER);
        workspaceArea->insertPanel(panelFive , panelSix     ,   Qt::Horizontal, AFTER);
        //workspaceArea->insertPanel(panelOne,      waterfall, Qt::Horizontal, AFTER);

        workspaceArea->addToPanelGroup(panelFive, panelSeven);
        workspaceArea->addToPanelGroup(panelFive , panelEight);
#else
		// Simpler layout for testing.
        WorkspacePanel* panelOne = getNewPanel("One");
        workspaceArea->addPanel(panelOne, Qt::Horizontal);

        WorkspacePanel* panelTwo = getNewPanel("Two");
        workspaceArea->addPanel(panelTwo, Qt::Horizontal);

        WorkspacePanel* panelThree = getNewPanel("Three");
        workspaceArea->addPanel(panelThree, Qt::Horizontal);
#endif
    }
    
    workspaceArea->endDeferLayout();
}


void MainWindow::createActions()
{
    newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
    newAct->setStatusTip(tr("Create a new file"));

    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

    closeAct = new QAction(tr("&Close"), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    closeAct->setStatusTip(tr("Close this window"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));

    copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));

    pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));

    newPanelAct = new QAction(tr("&New Panel"), this);
    newPanelAct->setShortcuts(QKeySequence::New);
    connect(newPanelAct, SIGNAL(triggered()), SLOT(newPanel()));

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));


    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
}

//! [implicit tr context]
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
//! [implicit tr context]
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(newPanelAct);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}


void 
MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr(""));
}


void 
MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}


void 
MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());

    // Try to save state
    QFile stateFile("./workspace.conf");
    if (stateFile.open(QIODevice::WriteOnly 
                     | QIODevice::Truncate 
                     | QIODevice::Text)) {

        QXmlStreamWriter stream(&stateFile);
        stream.setAutoFormatting(true);
        workspaceArea->saveState(stream, true);

        stateFile.close();
    }
}


MainWindow* 
MainWindow::findMainWindow(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QWidget *widget, qApp->topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin && mainWin->curFile == canonicalFilePath)
            return mainWin;
    }
    return 0;
}

