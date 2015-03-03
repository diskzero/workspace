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
#include "Workspace.h"

// System
#include <assert.h>
#include <iostream>

// Studio
//#include <logging/logging.h>

// Qt
#include <QApplication>
#include <QDragEnterEvent>
#include <QEvent>
#include <QFile>
#include <QLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QSettings>
#include <QSignalMapper>
#include <QStyleOptionDockWidget>
#include <QStylePainter>
#include <QtAlgorithms>
#include <QTimer>
#include <QToolBar>
#include <QtDebug>
#include <QtGlobal>
#include <QUrl>
#include <QXmlStreamWriter>

// Local
#include "WorkspaceItem.h"
#include "WorkspacePanel.h"

// Namespaces
using namespace workspace;

//=============================================================================
// struct SavedLayout
//=============================================================================
const QString Workspace::DefaultLayoutName("workspace_default_layout");

//-----------------------------------------------------------------------------
// SavedLayout::SavedLayout()
//
/// Construct a Layout structure
//-----------------------------------------------------------------------------
SavedLayout::SavedLayout(const QString& inName)
        :   name(inName)
{
}


//-----------------------------------------------------------------------------
// SavedLayout::writeState()
//
/// Write the layout to a stream
//-----------------------------------------------------------------------------
void 
SavedLayout::writeState(QXmlStreamWriter& inStream, WorkspaceArea* inArea)
{
    inArea->saveState(inStream, layout.second);
}


//-----------------------------------------------------------------------------
// SavedLayout::operator==(const SavedLayout& other) const
//
/// Defines equality of two layout on the basis of equality of their names.
//-----------------------------------------------------------------------------
bool SavedLayout::operator==(const SavedLayout& other) const
{
    return name == other.name;
}

//=============================================================================
// class Workspace
//=============================================================================

//-----------------------------------------------------------------------------
// Workspace::Workspace()
//
/// Construct a Workspace.
/// \param inTitle The title of the workspace that will be created.
//-----------------------------------------------------------------------------
Workspace::Workspace(const QString& inTitle)
    :   mWindowMapper(NULL),
        mMenuBar(NULL),
        mFileAction(NULL),
        mEditAction(NULL),
        mWindowsAction(NULL),
        mHelpAction(NULL),
#ifdef DEBUG
        mDebugAction(NULL),
#endif
        mPanelsMenu(NULL),
        mTitle(inTitle),
        mMainWindow(NULL),
        mSaveStateTimer(NULL),
        mPanelContentsMarginLeft(0),
        mPanelContentsMarginTop(0),
        mPanelContentsMarginRight(0),
        mPanelContentsMarginBottom(0),
        mPanelContentsMarginsSet(false),
        mLayoutDeferrals(0)

{
    // Create the main menu bar
    mMenuBar = new QMenuBar();

    mWindowMapper = new QSignalMapper(this);
    connect(mWindowMapper, SIGNAL(mapped(QWidget *)), this, SLOT(setActiveSubWindow(QWidget *)));

    mSaveStateTimer = new QTimer(this);
}


//-----------------------------------------------------------------------------
// Workspace::~Workspace()
//-----------------------------------------------------------------------------
Workspace::~Workspace()
{
    if (mMainWindow != NULL) {
        // Set mMainWindow to NULL before deleting it to prevent stale pointer
        // derefs in event filter
        QMainWindow* mw = mMainWindow;
        mMainWindow = NULL;
        delete mw;
    }
}


//-----------------------------------------------------------------------------
// Workspace::initialize()
//
/// Workspace environment initialization.
/// This method must be called after the class has been constructed.
/// This will handle the calling of virtual methods for menu creation
/// and other object initialization.
//-----------------------------------------------------------------------------
void
Workspace::initialize()
{
    // Add the workspace menu items

    //
    // File
    //
    mFileAction = getMenuBar()->addMenu(new QMenu(tr("&File")));
    //mFileAction->menu()->addAction("Quit", qApp, SLOT(quit()), QKeySequence("Ctrl+Q"));

    //
    // Edit
    //
    mEditAction = getMenuBar()->addMenu(new QMenu(tr("&Edit")));

    /*
    // Cut
    QAction* action = mEditAction->menu()->addAction("Cut");
    action->setShortcut(QKeySequence("Ctrl+X"));

    // Copy
    action = mEditAction->menu()->addAction("Copy");
    action->setShortcut(QKeySequence("Ctrl+C"));

    // Paste
    action = mEditAction->menu()->addAction("Paste");
    action->setShortcut(QKeySequence("Ctrl+V"));
    */

    //
    // Windows
    //
    mWindowsAction = getMenuBar()->addMenu(new QMenu(tr("&Windows")));
    mPanelsMenu = mWindowsAction->menu()->addMenu(tr("Panels"));
    mWindowsAction->menu()->addSeparator();

    //
    // Help
    //
    mHelpAction = getMenuBar()->addMenu(new QMenu(tr("&Help")));

#ifdef DEBUG
    //
    //  Debug
    //
    mDebugAction = getMenuBar()->addMenu(new QMenu(tr("&Debug")));
#endif
       
    // Call the virtual methods
    initMenus();

    initializeDockInfo();
    
    connect(mSaveStateTimer, SIGNAL(timeout()), this, SLOT(saveState()));
}

void
Workspace::initializeDockInfo()
{
    assert(mMainWindow == NULL);

    // Create the global window
    mMainWindow = new QMainWindow();
    mMainWindow->setObjectName(QLatin1String("WorkspaceWindow"));
    mMainWindow->setWindowTitle(mTitle);
    mMainWindow->installEventFilter(this);

    // Set up our docking options.
    mMainWindow->setDockOptions(QMainWindow::AnimatedDocks
                              | QMainWindow::AllowTabbedDocks
                              | QMainWindow::ForceTabbedDocks);

    // Create the global container
    WorkspaceArea* workspaceArea = new WorkspaceArea(mMainWindow);
    workspaceArea->installEventFilter(this);
#if 0
    connect(workspaceArea, 
            SIGNAL(tabRenameRequested(QTabWidget*)), 
            this, 
            SLOT(onTabRenameRequested(QTabWidget*)));
#endif

    // Connect the area to the panel creation slot
    connect(workspaceArea, 
            SIGNAL(updateCreatePanelsMenu(QMenu*)), 
            this, 
            SLOT(onUpdateCreatePanelsMenu(QMenu*)));

    // Set the central widget of the main window
    mMainWindow->setCentralWidget(workspaceArea);

    // Display the status bar
    (void) mMainWindow->statusBar();

    // Set up the menubar
    mMainWindow->setMenuBar(mMenuBar);

    updateWindowsMenu();
    connect(mWindowsAction->menu(), SIGNAL(aboutToShow()), this, SLOT(updateWindowsMenu()));

    // Add the entry for the global/default layout to the list of saved layouts.
    if (mLayouts.empty()) {
        SavedLayout globalLayout;
        mLayouts[Workspace::DefaultLayoutName] = globalLayout;
    }
}

//-----------------------------------------------------------------------------
// Workspace::startSaveStateTimer()
//-----------------------------------------------------------------------------
void
Workspace::startSaveStateTimer()
{
    if (mLayoutDeferrals == 0)
        mSaveStateTimer->start(2000);
}

//-----------------------------------------------------------------------------
// Workspace::eventFilter()
//-----------------------------------------------------------------------------
bool
Workspace::eventFilter(QObject* inObject, QEvent* inEvent)
{
    if (mMainWindow != NULL &&
        inObject == mMainWindow->centralWidget()){
        switch (inEvent->type()){
        case QEvent::DragEnter: {
            QDragEnterEvent* dragEnterEvent = static_cast<QDragEnterEvent*>(inEvent);
            if (dragEnterEvent->mimeData()->hasFormat(QLatin1String("text/uri-list"))) {
                dragEnterEvent->acceptProposedAction();
                return true;
            }
        }
            break;
        case QEvent::Drop: {
            QDropEvent* dropEvent = static_cast<QDropEvent*>(inEvent);
            if (!dropEvent->mimeData()->hasFormat(QLatin1String("text/uri-list")))
                return false;

            /*
            foreach(QUrl url, dropEvent->mimeData()->urls()) {
                if (!url.toLocalFile().isEmpty())
                    readInForm(url.toLocalFile());
            }
            */
            dropEvent->acceptProposedAction();

            return true;
        }
            break;

        case QEvent::Resize :
            startSaveStateTimer();
            break;
            
        default:;
        }
    } else if (inObject == mMainWindow){
        
        if (inEvent->type() == QEvent::Move)
            startSaveStateTimer();

    } else {
        // Not the central widget, probably a panel.
        switch (inEvent->type()){
        case QEvent::Move   :
        case QEvent::Resize :
        case QEvent::Close  :
            // Check on that "probably a panel" assumption
            assert(qobject_cast<WorkspacePanel*>(inObject) != NULL);
            startSaveStateTimer();
            break;

        default:;
        }
    }

    return false;
}


//-----------------------------------------------------------------------------
// Workspace::addWidget()
///
/// Add a widget to the Workspace, wrapping it with a WorkspacePanel.
/// This will create a docked panel in the central area.
/// \param inWidget The widget to add as a panel.
/// \param inGroup The panel group to add the panel to.
/// \param inName The name to use for the given widget.
/// \param isFloating If true create a floating panel.
//-----------------------------------------------------------------------------
void
Workspace::addWidget(QWidget* inWidget,
                     QWidget* inGroup, 
                     const QString& inName, 
                     bool isFloating)
{
    Q_ASSERT(inWidget != NULL);
    Q_ASSERT(mMainWindow != NULL);

    // Reparent this widget to ourself
    inWidget->setParent(mMainWindow);
    
    // So we can draw the background between panels
    inWidget->setAutoFillBackground(true);

    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(workspaceArea != NULL);

    WorkspacePanel* panel = new WorkspacePanel(inWidget->windowTitle(), workspaceArea);
    panel->installEventFilter(this);
 
    panel->setObjectName(inName);
    panel->setWidget(inWidget);

    // If the inWidget is a WorkspaceItem, then let it keep a pointer 
    // to its owning panel
    WorkspaceItem* wbItem = qobject_cast<WorkspaceItem*>(inWidget);
    if (NULL != wbItem) {
        wbItem->setOwningPanel(panel);
    }

    if (mPanelContentsMarginsSet && inWidget->layout() != NULL) {
        inWidget->layout()->setContentsMargins(
            mPanelContentsMarginLeft,
            mPanelContentsMarginTop,
            mPanelContentsMarginRight,
            mPanelContentsMarginBottom);
    }
    
    if (isFloating) {
        workspaceArea->addFloatingPanel(panel);
    } else {
        if (inGroup == NULL) {
            workspaceArea->addPanel(panel, Qt::Horizontal);
        } else {
            WorkspacePanel* theGroup = qobject_cast<WorkspacePanel*>(inGroup);
            Q_ASSERT(theGroup != NULL);
            workspaceArea->addToPanelGroup(theGroup, panel);           
        }
    }
}


/// When restoring open widgets and workspaceitems, you don't want the layout
/// adjusted until all the widgets have been added. Call this before adding
/// widgets or workspaceitems.
void
Workspace::beginDeferLayout()
{
    if (mLayoutDeferrals++ == 0){
        WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
        Q_ASSERT(workspaceArea != NULL);
        workspaceArea->beginDeferLayout();
    }
}

/// When restoring open widgets and workspaceitems, you don't want the layout
/// adjusted until all the widgets have been added. Call this after adding
/// widgets or workspaceitems.
void
Workspace::endDeferLayout()
{
    if (--mLayoutDeferrals == 0){
        WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
        Q_ASSERT(workspaceArea != NULL);
        workspaceArea->endDeferLayout();
    }
}


//-----------------------------------------------------------------------------
// Workspace::addWorkspaceItem()
///
/// Add the WorkspaceItem to the Workspace, wrapping it with a WorkspacePanel.
/// \param inWidget The item to add to the workspace.
/// \param inName The placeholder name to use for the given widget.
//-----------------------------------------------------------------------------
void
Workspace::addWorkspaceItem(WorkspaceItem* inWidget, const QString& inName)
{
    assert(inWidget != NULL);
    
    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(workspaceArea != NULL);

    WorkspacePanel* owningPanel = inWidget->getOwningPanel();
    
    // Artemis launch time or load layout time
    if (NULL == owningPanel) {
        // Set the active panel to NULL.  We are going to be resetting
        // all of the panels and a new active panel will need to be set.
        workspaceArea->setActivePanel(NULL);
    
        if (!mWorkspaceItems.contains(inWidget))
            mWorkspaceItems.append(inWidget);

        addWidget(inWidget, NULL, inName);

        // Check and see if we need to merge menus
        const WorkspaceItem::MenuMergeType menuMergeType = inWidget->getMenuMergeType();

        switch (menuMergeType) {
            case WorkspaceItem::ADD:
                addItemMenus(inWidget);
                break;

            case WorkspaceItem::MERGE:
                mergeItemMenus(inWidget);
                break;

            case WorkspaceItem::REMOVE:
                removeItemMenus(inWidget);
                break;

            case WorkspaceItem::REPLACE:
                replaceItemMenus(inWidget);
                break;

            case WorkspaceItem::NONE:
            default:
                // No menus to merge
                break;
        }
    } else {
        //This is neither artemis-launch time, nor load-layout time
        //The user must have clicked View->Plugin
        if (owningPanel->isFloating() && !owningPanel->isVisible()) {
            workspaceArea->addFloatingPanel(owningPanel);
        } else {
            if (!owningPanel->isVisible()) {
                workspaceArea->addPanel(owningPanel, Qt::Horizontal);
            }
        }
    }
}


//-----------------------------------------------------------------------------
// Workspace::removeWorkspaceItem()
///
/// Remove the widget from the Workspace dock.
/// \param inWidget The widget to remove from the Workspace dock.
//-----------------------------------------------------------------------------
void
Workspace::removeWorkspaceItem(WorkspaceItem* inWidget)
{
    Q_ASSERT(inWidget != NULL);

    // Remove the docked widget from the list and the window                
    if (!mWorkspaceItems.removeOne(inWidget))
        return;

    if (mMainWindow == NULL)
        return;

    WorkspacePanel* thePanel = qobject_cast<WorkspacePanel*>(inWidget->parentWidget());
    WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    
    // The WorkspaceArea may be NULL if the central area widget has already been destroyed
    if (theArea != NULL && thePanel != NULL) {
        theArea->removePanel(thePanel);
        thePanel->setWidget(NULL);
        delete thePanel;
    }
}


//-----------------------------------------------------------------------------
// Workspace::setDefaultStateFile()
//
/// Sets the file to restore state from if no user state files are found.
/// Presumeably, this file is built-into the program, or perhaps shipped
/// alongside it, and is therefore not writeable.
//-----------------------------------------------------------------------------
void
Workspace::setDefaultStateFile(const QString& inFile)
{
    mDefaultStateFile = inFile;
}


//-----------------------------------------------------------------------------
// Workspace::setUserStateFile()
//
/// Sets the file used for saving and restoring state. If the file doesn't
/// exist, state will be restored from the "built-in" state file.
/// \sa setDefaultStateFile
//-----------------------------------------------------------------------------
void
Workspace::setUserStateFile(const QString & file)
{
    mUserStateFile = file;
}

//-----------------------------------------------------------------------------
// Workspace::getOpenPanelsList()
//
/// Get the list of all open panels
/// \param outList The list of open panels
//-----------------------------------------------------------------------------
void
Workspace::getOpenPanelsList(WorkspacePanelList& outList) const
{
    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(workspaceArea != NULL);
    return workspaceArea->getPanelList(outList);
}

//-----------------------------------------------------------------------------
// Workspace::updateWindowMenu()
//
/// Populate the Windows menu
//-----------------------------------------------------------------------------
void
Workspace::updateWindowsMenu()
{
    Q_ASSERT(mPanelsMenu != NULL);

    // Clear the list of panels, which may be stale.
    mPanelsMenu->clear();

    // Get the list of all open panels
    WorkspacePanelList windows;
    getOpenPanelsList(windows);

    int index = 0;
    Q_FOREACH(WorkspacePanel* panel, windows){
        QWidget* child = panel->widget();

        QString text;
        if (index < 9) {
            text = tr("&%1 %2").arg(index + 1)
                               .arg(child->windowTitle());
        } else {
            text = tr("%1 %2").arg(index + 1)
                              .arg(child->windowTitle());
        }

        QAction* action = mPanelsMenu->addAction(text);
        connect(action, SIGNAL(triggered()), mWindowMapper, SLOT(map()));
        mWindowMapper->setMapping(action, panel);
        ++index;
    }
}


//-----------------------------------------------------------------------------
// Workspace::onTabRenameRequested()
//
/// Virtual slot to allow interactive renaming of the current tab.
/// Override to handle renaming.
/// \param inTabBar The TabBar.
//-----------------------------------------------------------------------------
void
Workspace::onTabRenameRequested(QTabWidget* inTabBar)
{
    Q_UNUSED(inTabBar);
}


//-----------------------------------------------------------------------------
// Workspace::onUpdateCreatePanelsMenu()
//
/// Virtual slot to populate the panel creation menu.
/// Override to populate the menu.
/// \param inMenu The menu to populate.
//-----------------------------------------------------------------------------
void
Workspace::onUpdateCreatePanelsMenu(QMenu* inMenu)
{
    Q_UNUSED(inMenu);
}


//-----------------------------------------------------------------------------
// Workspace::addItemMenus()
//
/// Add the WorkspaceItems menus to the Workspace menu bar.
/// \param inItem The WorkspaceItem whose menus will be added.
//-----------------------------------------------------------------------------
void
Workspace::addItemMenus(WorkspaceItem* inItem)
{
    Q_ASSERT(inItem != NULL);

    // Get the items menus.
    const WorkspaceItem::MenuList& menuList = inItem->getMenuList();
    Q_FOREACH(QMenu* menu, menuList) {
        // Add the item to the end of the menubar, before the Windows and Help menu.
        mMenuBar->insertMenu(mWindowsAction, menu);
    }

    // Now add the item to the windows menu
    if (mWindowsAction != NULL) {

    }
}


//-----------------------------------------------------------------------------
// Workspace::mergeItemMenus()
//
/// Merge the WorkspaceItems menus intto the Workspace menu bar.
/// \param inItem The WorkspaceItem whose menus will be merged.
//-----------------------------------------------------------------------------
void
Workspace::mergeItemMenus(WorkspaceItem* /*inItem*/)
{

}


//-----------------------------------------------------------------------------
// Workspace::removeItemMenus()
//
/// Remove the WorkspaceItems menus from the Workspace menu bar.
/// \param inItem The WorkspaceItem whose menus will be removed.
//-----------------------------------------------------------------------------
void
Workspace::removeItemMenus(WorkspaceItem* /*inItem*/)
{

}


//-----------------------------------------------------------------------------
// Workspace::replaceItemMenus()
//
/// Replace the WorkspaceItems menus in the Workspace menu bar.
/// \param inItem The WorkspaceItem whose menus replacing the
/// Workspace menu items.
//-----------------------------------------------------------------------------
void
Workspace::replaceItemMenus(WorkspaceItem* /*inItem*/)
{

}

//-----------------------------------------------------------------------------
// Workspace::restoreState()
//
/// Restore window state from one of the files specified by setDefaultStateFile
/// or setUserStateFile. Note that state will not be saved, until after
/// restoreState has been called.
//-----------------------------------------------------------------------------
void
Workspace::restoreState()
{
    bool stateRestored = false;
    bool restoreAttempted = false;

    if (!mUserStateFile.isEmpty()) {
        QFile stateFile(mUserStateFile);
        if (stateFile.exists()) {
            restoreAttempted = true;
            stateRestored = restoreState(&stateFile);
            // TODO mrequenes If restoreState failed, we should copy the old
            // state file to a safe place to prevent it from being overwritten.
        }
    }
    
    if (!stateRestored && !mDefaultStateFile.isEmpty()) {
        // The default state file may be a built-in resource
        QFile stateFile(mDefaultStateFile);
        if (stateFile.exists()) {
            restoreAttempted = true;
            stateRestored = restoreState(&stateFile);
        }
    }

    if (restoreAttempted && !stateRestored) {
        //LOG_WARN("Error restoring window layout");
    }
}

// Private function
bool
Workspace::restoreState(QIODevice* inFile)
{
    Q_ASSERT(inFile != NULL);

    // Open the file for reading
    if (!inFile->open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    // Clear out the cached layout.
    mSavedLayout.second.clear();

    // Get the area
    WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(theArea != NULL);
    
    // Create the XML stream
    QXmlStreamReader xmlStream(inFile);
    
    // Get the Workspace area configuration
    const bool theResult = theArea->restoreState(xmlStream, mSavedLayout);

    // Done with the file
    inFile->close();

    return theResult;
#if 0
    quint8 marker, version;
    stream >> marker;
    if (marker != WorkspaceMarker) {
        return false;
    }

    stream >> version;
    if (version != StateFileVersion) {
        if (version != OldStateFileVersion) {
            return false;
        } else {
            QRect geom;
            stream >> geom;
            if(geom.isValid())
                mMainWindow->setGeometry(geom);
            WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
            return workspaceArea->restoreState(stream);
        }
    }

    bool stateRestored;
    SavedLayout* layout = getGlobalLayout();
    
    // Read in the global layout
    layout->readState(stream);
    
    // Read in other layout states if they exist
    while(!stream.atEnd()) {
        SavedLayout otherLayouts;
        otherLayouts.readState(stream);
        mLayoutList.push_back(otherLayouts);
    }

    // Switch to the default state. 
    stateRestored = switchLayout(DefaultLayoutName);
    return stateRestored;
#endif
}


//-----------------------------------------------------------------------------
// Workspace::restoreComplete()
//
/// Call this method after calling Workspace::restoreState() and any
/// application specific behavior.  This will ensure that the current
/// tabs and any other state needed to be restored will be applied
/// to the fully restored Workspace.
//-----------------------------------------------------------------------------
void
Workspace::restoreComplete()
{
    WorkspaceArea* workspaceArea = 
        qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(workspaceArea != NULL);

    workspaceArea->restoreComplete();
}


//-----------------------------------------------------------------------------
// Workspace::switchLayout(QString layoutName)
//
/// Switch window to a particular layout.
//-----------------------------------------------------------------------------
bool
Workspace::switchLayout(const QString& inLayoutName)
{
    bool result = false;
    SavedLayout theLayout;
    if (inLayoutName.isEmpty()) {
        getGlobalLayout(theLayout);
        result = true;
    } else {
        result = getSavedLayout(inLayoutName, theLayout);
    }
    
    assert(result && "The layout to switch to must be found.");

#if 0    
    if(layout->geometry.isValid()) {
        mMainWindow->setGeometry(layout->geometry);
    }

    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    QDataStream wbaStream(layout->state);
    return workspaceArea->restoreState(wbaStream);
#endif

    return false;
}

//-----------------------------------------------------------------------------
// Workspace::saveState()
//
/// Save default window state
//-----------------------------------------------------------------------------
void
Workspace::saveState()
{
    // If in transitional state, then do not save state
    if(mLayoutDeferrals > 0)
        return;

    saveState(Workspace::DefaultLayoutName);
}

//-----------------------------------------------------------------------------
// Workspace::saveState()
//
/// Save the named window state, to the file specified by setUserStateFile.
/// \param inLayoutName The name of the layout to save.
//-----------------------------------------------------------------------------
static const char* kWorkspaceElement = "workspace";
static const char* kWorkspaceLayoutElement = "layout";
static const char* kWorkspaceVersionAttribute = "version";
static const char* kWorkspaceLayoutNameAttribute = "name";

void
Workspace::saveState(const QString& inLayoutName)
{
    if (mUserStateFile.isEmpty())
        return;

    Q_ASSERT(!inLayoutName.isEmpty());

    // Update the said layout to hold the current layout
    updateSavedLayout(inLayoutName);
    
    WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(theArea != NULL);

    QFile stateFile(mUserStateFile);
    if (stateFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {

        // Create the xml writer and configure it
        QXmlStreamWriter stream(&stateFile);
        stream.setAutoFormatting(true);

        stream.writeStartDocument();
    
        // Begin the saved layouts
        stream.writeStartElement(kWorkspaceElement);

        // Write out the current version
        stream.writeAttribute(kWorkspaceVersionAttribute, QString::number(StateFileVersion));

        // Save all layouts by making them write to our stream
        LayoutIterator iter(mLayouts);
        while (iter.hasNext()) {

            // Get the layout
            iter.next();
            SavedLayout& layout = iter.value();

            // Write out the layout
            stream.writeStartElement(kWorkspaceLayoutElement);
            stream.writeAttribute(kWorkspaceLayoutNameAttribute, iter.key());

            layout.writeState(stream, theArea);

            // Done writing out this layout
            stream.writeEndElement(); // kWorkspaceLayoutElement
        }

        // Done saving out the workspace layout
        stream.writeEndElement(); // kWorkspaceElement

        // Done with the file
        stream.writeEndDocument();
        stateFile.close();
        
    } else {
        //LOG_WARN_FIRST_N(1, "error saving state to file: " << mUserStateFile.toStdString().c_str());
    }
    
    mSaveStateTimer->stop();
}


//-----------------------------------------------------------------------------
// Workspace::layoutExists()
//
/// Check for the existence of the layout
/// \param inLayoutName The name of the layout.
/// \result True if the layout was found.
//-----------------------------------------------------------------------------
bool
Workspace::layoutExists(const QString& inLayoutName) const
{
    return mLayouts.contains(inLayoutName);
}


//-----------------------------------------------------------------------------
// Workspace::getSavedLayout()
//
/// Get the saved layout for the name.
/// \param inLayoutName The name of the layout to save.
/// \param outLayout The layout to populate.
/// \result True if the layout was found.
//-----------------------------------------------------------------------------
bool
Workspace::getSavedLayout(const QString& inLayoutName, SavedLayout& outLayout) 
{
    LayoutIterator iter(mLayouts);
    while (iter.hasNext()) {
        iter.next();

        if (iter.key() == inLayoutName) {        
            outLayout = iter.value();
            return true;
        }
    }
        
    return false;
}


//-----------------------------------------------------------------------------
// Workspace::getGlobalLayout()
//
/// Get a pointer to the global layout.
//-----------------------------------------------------------------------------
void
Workspace::getGlobalLayout(SavedLayout& outLayout)
{
    const bool foundIt = getSavedLayout(Workspace::DefaultLayoutName, outLayout); 
    assert(foundIt && "Default layout must be present. This code should not be reached");
}


//-----------------------------------------------------------------------------
// Workspace::updateSavedLayout()
//
/// Update the layout with the given name, with the current geometry
/// \param inLayoutName The name of the layout to save.
//-----------------------------------------------------------------------------
void
Workspace::updateSavedLayout(const QString& inLayoutName)
{
    Q_ASSERT(!inLayoutName.isEmpty());
    
    SavedLayout theLayout;

    // Get the window geometry
    theLayout.geometry = mMainWindow->geometry();

    // Save the current state into the layout structure.
    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    workspaceArea->saveState(theLayout.layout);

    // Update the layout
    mLayouts[inLayoutName] = theLayout;
}


//-----------------------------------------------------------------------------
// Workspace::setActiveSubWindow()
//
/// Set the window as the active subwindow
/// \param inWindow The new active subwindow
//-----------------------------------------------------------------------------
void
Workspace::setActiveSubWindow(QWidget* window)
{
    // This code copied from UUi::activateWindow
    if (window->isHidden()){
        window->show();
    } else if (window->isMinimized()){
        window->showNormal();
    }
    window->activateWindow();
    window->raise();
}

//-----------------------------------------------------------------------------
// Workspace::setPanelContentsMargins()
//
/// Sets optional contents margins to be applied to all WorkspaceItems and
/// QWidgets added to the workspace. I.e., all items passed to
/// 'addWorkspaceItem' and all widgets passed to 'addWidget'. This is mainly a
/// convenience -- the caller could do this himself, before passing in the
/// widgets and items.
///
/// If this function in not called, then the contents margins of the items
/// and widgets are left as is.
///
/// \sa QLayout::setContentsMargins
//-----------------------------------------------------------------------------
void
Workspace::setPanelContentsMargins(int left, int top, int right, int bottom)
{
    mPanelContentsMarginLeft = left;
    mPanelContentsMarginTop = top;
    mPanelContentsMarginRight = right;
    mPanelContentsMarginBottom = bottom;
    mPanelContentsMarginsSet = true;
}


//-----------------------------------------------------------------------------
// Workspace::getLayoutNames()
//
/// Get a list of all layout names.
/// \param outNames List of names to be filled.
//-----------------------------------------------------------------------------
void
Workspace::getLayoutNames(QStringList& outNames)
{
   LayoutConstIterator iter(mLayouts);
    while (iter.hasNext()) {
        iter.next();
        outNames.push_back(iter.key());
    }
}


//-----------------------------------------------------------------------------
// Workspace::deleteLayout(layoutName)
//
/// Remove named layout from mLayoutList
/// \param inName The name of the layout to remove.
/// \result True if the layout was removed.
//-----------------------------------------------------------------------------
bool
Workspace::deleteLayout(const QString& inName)
{
    // Don't allow the default layout to be removed
    if (inName == Workspace::DefaultLayoutName)
        return false;

    LayoutIterator iter(mLayouts);
    while (iter.hasNext()) {
        iter.next();

        if (iter.key() == inName) {
            iter.remove();
            return true;
        }
    }

    return false;
}


//-----------------------------------------------------------------------------
// Workspace::renameLayout()
//
/// Rename the layout with the new name.
/// \param inName The name of the layout
/// \param inName The new name of the layout.
//-----------------------------------------------------------------------------
bool
Workspace::renameLayout(const QString& inName, const QString& inNewName)
{
    LayoutIterator iter(mLayouts);
    while (iter.hasNext()) {
        iter.next();

        if (iter.key() == inName) {

            // TODO gragan: This code needs to check for duplicates.
            SavedLayout layout = iter.value();
            layout.name = inNewName;       
            iter.remove();
            mLayouts[inNewName] = layout;
            return true;
        }
    }

    return false;
}


//-----------------------------------------------------------------------------
// Workspace::addPanelCreationAction()
//
/// Add an action to be used by the workspace to create new panels.
/// This action will be added to action menu that is present
/// on each panels tab bar, assuming the action menu is visible.
/// The ownership of the action is transferred to the Workspace.
/// \param inAction The QAction to add.
//-----------------------------------------------------------------------------
void
Workspace::addPanelCreationAction(QAction* inAction)
{
	Q_ASSERT(inAction != NULL);
	
	mCreationActionList.push_back(inAction);
}


//-----------------------------------------------------------------------------
// Workspace::closePanels()
//
/// Close all workspace panels.
//-----------------------------------------------------------------------------
void
Workspace::closePanels()
{
    WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(theArea != NULL);
    theArea->closePanels();
    delete theArea;
}


//-----------------------------------------------------------------------------
// Workspace::renamePanel()
//
/// Attempt to rename the active panel of the tab group.
/// \param inName The requested name.
/// \result True if the rename succeeded.
//-----------------------------------------------------------------------------
bool
Workspace::renamePanel(QTabWidget* inTabBar, const QString& inName)
{
    Q_ASSERT(inTabBar != NULL);

    // Get the area
    WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(mMainWindow->centralWidget());
    Q_ASSERT(theArea != NULL);

    // Ask the area to do the rename.
    return theArea->renamePanel(inTabBar, inName);
}


//-----------------------------------------------------------------------------
// Workspace::setWindowTitle()
//
/// Set the window title.
/// \param inTitle The new title.
//-----------------------------------------------------------------------------
void
Workspace::setWindowTitle(const QString& inTitle)
{
    // Cache the title
    mTitle = inTitle;

    // Update the main window title.
    if (mMainWindow != NULL)
        mMainWindow->setWindowTitle(mTitle);
}


//-----------------------------------------------------------------------------
// Workspace::getWorkspacePanel()
//
/// Get the workspace panel for the widget.
/// \param inWidget The widget to find the panel for.
/// \result The panel, or NULL if none if found.
//-----------------------------------------------------------------------------
WorkspacePanel*
Workspace::getWorkspacePanel(QWidget* inWidget) const
{
    Q_ASSERT(inWidget != NULL);

    WorkspacePanelList panelList;
    getOpenPanelsList(panelList);
    Q_FOREACH(WorkspacePanel* thePanel, panelList) {
        if (thePanel->widget() == inWidget)
            return thePanel;
    }

    return NULL;
}
