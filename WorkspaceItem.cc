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
#include "WorkspaceItem.h"

// System
#include <assert.h>

// Qt
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QMenu>
#include <QVBoxLayout>

// Local 
#include "Workspace.h"

// Namespaces
using namespace workspace;

//=============================================================================
// class WorkspaceItem
//=============================================================================

// Set up the slot name statics. These must match the slots in the class.
const char* WorkspaceItem::Copy("onCopy");
const char* WorkspaceItem::Cut("onCut");
const char* WorkspaceItem::Paste("onPaste");
const char* WorkspaceItem::Duplicate("onDuplicate");               
const char* WorkspaceItem::Delete("onDelete");               
const char* WorkspaceItem::SelectAll("onSelectAll");               
const char* WorkspaceItem::SelectNone("onSelectNone");                       
const char* WorkspaceItem::InvertSelection("onInvertSelection");    


//-----------------------------------------------------------------------------
// WorkspaceItem::WorkspaceItem()
// 
/// Creates a window that can be added to a workspace. Once the item
/// is added to the workspace, it will appear as a docked item if the
/// Workspace is set to docked mode.  If the workspace is set to top level
/// mode, all items will appear as top level windows.
/// Inherited classes can set up a menu hierarchy. Based on the menu merge
/// type, the Workspace window will perform a certain type of menu merging.
//-----------------------------------------------------------------------------
WorkspaceItem::WorkspaceItem(Workspace* inWorkspace, 
                             QWidget* inParent, 
                             Qt::WindowFlags inFlags)
    : QWidget(inParent, inFlags | Qt::Window),
      mWorkspace(inWorkspace),
      mSaveSettings(false),
      mFloating(false),
      mMenuMergeType(NONE),
      mLayout(NULL),
      mCentralWidget(NULL),
      mOwningPanel(NULL)
{
    Q_ASSERT(mWorkspace != NULL);

    // Set the same icon as the application
    setWindowIcon(qApp->windowIcon());

    mLayout = new QVBoxLayout(this);

    // Create the show/hide action. This will be used by the Windows
    // menu item in the Workspace window.
    mAction = new QAction(this);
    mAction->setShortcutContext(Qt::ApplicationShortcut);
    mAction->setText(windowTitle());
    mAction->setCheckable(true);
    connect(mAction, SIGNAL(triggered(bool)), this, SLOT(showMe(bool)));
}


//-----------------------------------------------------------------------------
// WorkspaceItem::~WorkspaceItem()
//-----------------------------------------------------------------------------
WorkspaceItem::~WorkspaceItem()
{
    if (mWorkspace != NULL)
        mWorkspace->removeWorkspaceItem(this);
}



//-----------------------------------------------------------------------------
// WorkspaceItem::setTitle()
//
/// Emit the titleSet signal. This will cause the title of the item
/// to be updated.
//-----------------------------------------------------------------------------
void
WorkspaceItem::setTitle(const QString& inTitle)
{
    Q_EMIT titleSet(inTitle);
}


//-----------------------------------------------------------------------------
// WorkspaceItem::showMe()
//-----------------------------------------------------------------------------
void
WorkspaceItem::showMe(bool inShow)
{
    QWidget* target = parentWidget() == 0 ? this : parentWidget();

    if (inShow)
        target->setWindowState(target->windowState() & ~Qt::WindowMinimized);

    target->setVisible(inShow);
}


//-----------------------------------------------------------------------------
// WorkspaceItem::showEvent()
//-----------------------------------------------------------------------------
void 
WorkspaceItem::showEvent(QShowEvent* inEvent)
{
    Q_UNUSED(inEvent);

    const bool blocked = mAction->blockSignals(true);
    mAction->setChecked(true);
    mAction->blockSignals(blocked);
}


//-----------------------------------------------------------------------------
// WorkspaceItem::hideEvent()
//-----------------------------------------------------------------------------
void 
WorkspaceItem::hideEvent(QHideEvent* inEvent)
{
    Q_UNUSED(inEvent);

    bool blocked = mAction->blockSignals(true);
    mAction->setChecked(false);
    mAction->blockSignals(blocked);
}


//-----------------------------------------------------------------------------
// WorkspaceItem::getAction()
// 
/// Return the QAction associated with this item.
/// Note: this currently is not being called. Potentially, it could be used
/// to populate an application 'View' menu. I.e. 'View Plugin-X'
/// \result The item's QAction
//-----------------------------------------------------------------------------
QAction* 
WorkspaceItem::getAction() const
{
    return mAction;
}


//-----------------------------------------------------------------------------
// WorkspaceItem::changeEvent()
//-----------------------------------------------------------------------------
void 
WorkspaceItem::changeEvent(QEvent* inEvent)
{
    switch (inEvent->type()) {
        case QEvent::WindowTitleChange:
            mAction->setText(windowTitle());
            break;
            
        case QEvent::WindowIconChange:
            mAction->setIcon(windowIcon());
            break;
            
        default:
            break;
    }
    
    QWidget::changeEvent(inEvent);
}


//-----------------------------------------------------------------------------
// WorkspaceItem::getWorkspace()
// 
/// Return the Workspace that contains this item.
/// \result The item's Workspace/
//-----------------------------------------------------------------------------
Workspace* 
WorkspaceItem::getWorkspace() const
{
    return mWorkspace;
}


//-----------------------------------------------------------------------------
// WorkspaceItem::geometryHint()
//
/// Get a bounding rectangle hint, used for layout.
/// \result The geometry hint rectangle.
//-----------------------------------------------------------------------------
QRect 
WorkspaceItem::geometryHint() const
{
    return QRect();
}


//-----------------------------------------------------------------------------
// WorkspaceItem::closeEvent()
//-----------------------------------------------------------------------------
void 
WorkspaceItem::closeEvent(QCloseEvent* inEvent)
{
/*
    if (mSaveSettings) {
        inEvent->setAccepted(workspace()->handleClose());
        if (inEvent->isAccepted() && mWorkspace->mainWindow() == this)
            QMetaObject::invokeMethod(mWorkspace, "quit", Qt::QueuedConnection); 
    } else {
        QWidget::closeEvent(ev);
    }
*/
    QWidget::closeEvent(inEvent);
}


//-----------------------------------------------------------------------------
// WorkspaceItem::saveSettingsOnClose()
//
/// Return the state of the item's save settings on close.
/// \result True, is the item will save settings when closed.
//-----------------------------------------------------------------------------
bool 
WorkspaceItem::saveSettingsOnClose() const
{
    return mSaveSettings;
}


//-----------------------------------------------------------------------------
// WorkspaceItem::setSaveSettingsOnClose()
//
/// Set the state of the item's save settings on close.
/// \param inSave Set if the item will save settings when closed.
//-----------------------------------------------------------------------------
void 
WorkspaceItem::setSaveSettingsOnClose(bool inSave)
{
    mSaveSettings = inSave;
}


//-----------------------------------------------------------------------------
// WorkspaceItem::setMenuMergeType()
//
/// Set the type of menu merging that will occur when the WorkspaceItem
/// is added to a Workspace.
/// \param inMenuMergeType The type of menu merging that will occur.
///
/// Menu merge types:
/// 
///      ADD         The menu item is added to the collection of existing
///                  menu item objects. The menu will be added to the end
///                  of the menu bar items, but before the Help menu item.
///
///      MERGE       All submenu items of the menu are merged with those
///                  of existing menu item objects at the same position
///                  in a merged menu.
///                  
///      REMOVE      The menu item is not included in a merged menu.
///      
///      REPLACE     The menu item replaces an existing menu item
///                  at the same position in a merged menu.
//-----------------------------------------------------------------------------
void 
WorkspaceItem::setMenuMergeType(MenuMergeType inMenuMergeType)
{
    mMenuMergeType = inMenuMergeType;
}


//-----------------------------------------------------------------------------
// WorkspaceItem::addMenu()
//
/// Add a menu item associated with this item.
/// \param inTitle The title of the menu to add.
/// \result The QMenu that was added.
//-----------------------------------------------------------------------------
QMenu* 
WorkspaceItem::addMenu(const QString& inTitle)
{
    QMenu* theMenu = new QMenu(inTitle);
    mMenuList.push_back(theMenu);
    
    return theMenu;
}


//-----------------------------------------------------------------------------
// WorkspaceItem::getDockPlacementHint()
//
/// Return a placement hint for dock layout. Override to provide a hint
/// specific to the widget.
/// \result The dock area placementh hint.
//-----------------------------------------------------------------------------
Qt::DockWidgetArea 
WorkspaceItem::getDockPlacementHint() const
{
    return Qt::AllDockWidgetAreas;
}

void
WorkspaceItem::setCentralWidget(QWidget* w)
{
    if (mCentralWidget && mCentralWidget != w){
        mCentralWidget->hide();
        mCentralWidget->deleteLater();
    }
    mCentralWidget = w;
    mLayout->addWidget(mCentralWidget);
}

QWidget*
WorkspaceItem::centralWidget() const
{
    return mCentralWidget;
}

WorkspacePanel*
WorkspaceItem::getOwningPanel() const
{
    return mOwningPanel;
}

void
WorkspaceItem::setOwningPanel(WorkspacePanel* panel)
{
    mOwningPanel = panel;
}

//=============================================================================
// class WorkspacePlugin
//=============================================================================

//-----------------------------------------------------------------------------
// WorkspacePlugin::WorkspacePlugin()
//
/// Create a workspace plugin.  This is an interface class that must
/// be subclassed.
//-----------------------------------------------------------------------------
WorkspacePlugin::WorkspacePlugin()
    :   mItemList()
{
}

//-----------------------------------------------------------------------------
// WorkspacePlugin::~WorkspacePlugin()
//
/// Destructor. Delete any items remaining in the item list.
//-----------------------------------------------------------------------------
WorkspacePlugin::~WorkspacePlugin()
{
    qDeleteAll(mItemList);
    mItemList.clear();
}

//-----------------------------------------------------------------------------
// WorkspacePlugin::handleItemDestroyed()  [slot]
// 
/// The item has been destroyed. Remove it from the list.
/// \param inObject The destroyed item to remove.
//-----------------------------------------------------------------------------
void 
WorkspacePlugin::slotItemDestroyed(QObject* inObject)
{
    WorkspaceItem* item = static_cast<WorkspaceItem*>(inObject);
    if (item) {
        int index = mItemList.indexOf(item);
        if (index < 0) return;

        mItemList.replace(index, NULL);
    }
}

//-----------------------------------------------------------------------------
// WorkspacePlugin::addItem()
// 
/// Do any common processing to item and add to the list. This is called by
/// subclasses after they have created the item.
//-----------------------------------------------------------------------------
void
WorkspacePlugin::addItem(WorkspaceItem* item)
{
    // plugin created items will delete on close
    item->setAttribute(Qt::WA_DeleteOnClose);

    // insert into list
    int index = mItemList.indexOf(NULL);
    if (index < 0) {
        index = mItemList.size();
        mItemList.append(item);
    } else {
        mItemList.replace(index, item);
    }

    // number the window titles
    if (index > 0) {
        item->setWindowTitle(QString("%1 (%2)").arg(item->windowTitle()).arg(index+1));
    }

    connect(item, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotItemDestroyed(QObject*)));
}
