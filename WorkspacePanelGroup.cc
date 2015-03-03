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
#include "WorkspacePanelGroup.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QToolButton>
#include <QtDebug>

#ifndef Q_WS_MAC
// Needed for window title drag key modifier detection
#include <X11/Xlib.h>
#endif

// Local
#include "WorkspaceArea.h"
#include "WorkspaceLayout.h"
#include "WorkspacePanel.h"
#include "WorkspaceTabBar.h"
#include "WorkspaceUtils.h"


//=============================================================================
// class WorkspacePanelGroupDragWidget
//=============================================================================
static const QSize kDraggerSize(8, 23);

WorkspacePanelGroupDragWidget::WorkspacePanelGroupDragWidget(WorkspacePanelGroup* inParent)
   :    QFrame(inParent)
   ,    mPanelGroup(inParent)
{
	// Set up the resizing characteristics
   setMinimumSize(kDraggerSize);
   setMaximumSize(kDraggerSize);
   QSizePolicy resizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   setSizePolicy(resizePolicy);
}    


WorkspacePanelGroupDragWidget::~WorkspacePanelGroupDragWidget()
{
}


void
WorkspacePanelGroupDragWidget::paintEvent(QPaintEvent* inEvent)
{
    // Paint the default control.
    QFrame::paintEvent(inEvent);
    
    // Paint the drag thumb.
    QStylePainter thePainter(this);
    
    // Set up some spacing values so the dragger is spaced
    // correctly with the group menu.
    const int spacing = 2;
    const int offsetX = 0;
    const int offsetY = 3;

    thePainter.setPen(palette().background().color().darker());
        for (int x = 2 + offsetX; x <= 4 + offsetX; x += spacing) {
            for (int y = offsetY; y <= 14 + offsetY; y += 2) {
                thePainter.drawPoint(x, y);
        }
    }
    
    thePainter.setPen(palette().background().color().lighter(110));
    for (int x = 3 + offsetX; x <= 5 + offsetX; x += spacing) {
        for (int y = offsetY; y <= 14 + offsetY; y += 2) {
            thePainter.drawPoint(x, y);
        }
    }
} 


//=============================================================================
// class WorkspacePanelGroup
//=============================================================================
static const int kFontSize = 11;

//-----------------------------------------------------------------------------
// WorkspacePanelGroup::WorkspacePanelGroup()
//-----------------------------------------------------------------------------
WorkspacePanelGroup::WorkspacePanelGroup(QWidget* inParent)
    :   QTabWidget(inParent)
    ,   mTabBar(NULL)
    ,   mPanelsMenu(NULL)
    ,   mClosePanelAction(NULL)
    ,   mUndockPanelAction(NULL)
    ,   mUndockGroupAction(NULL)
    ,   mDragWidget(NULL)
    ,   mCreatePanelsMenu(NULL)
    ,   mActive(false)

{
    // Create the tab bar and configure it
    mTabBar = new WorkspaceTabBar(this);
    mTabBar->setDrawBase(true);
    mTabBar->setElideMode(Qt::ElideRight);
    mTabBar->setExpanding(false);
    mTabBar->setMouseTracking(true);
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setUsesScrollButtons(false);
    setTabBar(mTabBar);

	// Add the corner widget
	initPanelsMenu();
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::~WorkspacePanelGroup()
//-----------------------------------------------------------------------------
WorkspacePanelGroup::~WorkspacePanelGroup()
{
}



//-----------------------------------------------------------------------------
// WorkspacePanelGroup::paintEvent()
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::paintEvent(QPaintEvent* inEvent)
{
    // Do the default fill
    QTabWidget::paintEvent(inEvent);

    QStylePainter thePainter(this);

	{
		// TODO gragan:  How can we theme this? Ask David J.
		const QColor& button = palette().button().color();
	    QColor dark;
	    dark.setHsv(button.hue(),
	                qMin(255, (int)(button.saturation() * 1.9)),
	                qMin(255, (int)(button.value() * 0.7)));
		
	    QRect backgroundRect = rect();
	    backgroundRect.setHeight(tabBar()->rect().height());
		
		// Draw the gradient fill to look like a non-active tab
		QLinearGradient gradient(backgroundRect.topLeft(), backgroundRect.bottomLeft());
	    gradient.setColorAt(0, palette().background().color());
	    gradient.setColorAt(1, dark.lighter(120));
        thePainter.fillRect(backgroundRect.adjusted(0, 3, 0, -2), gradient);
	    
	    // Outline the fill
		thePainter.setBrush(Qt::NoBrush);
		thePainter.setPen(palette().background().color().darker());
        thePainter.drawRect(backgroundRect.adjusted(0, 3, -1, -2));
	}

    // Draw the bottom border so the entire area looks like a QTabWidget.
    QColor button = palette().button().color();
    QColor darkOutline;
      darkOutline.setHsv(button.hue(),
                qMin(255, (int)(button.saturation() * 2.0)),
                qMin(255, (int)(button.value() * 0.6)));
    QColor highlight = palette().highlight().color();
    
    const QRect& frame = rect();

    if (mActive) {
        // Draw the bottom line
        QLine topLine = QLine(QPoint(0, frame.bottom()), 
                              QPoint(frame.right(), frame.bottom()));
        thePainter.setPen(WorkspaceArea::ActivePanelColor);
        thePainter.drawLine(topLine);
    
    } else {
        // Draw the top line of the bottom highlight
        QLine topLine = QLine(QPoint(0, frame.bottom() - 1), 
                              QPoint(frame.right(), frame.bottom() - 1));
        thePainter.setPen(darkOutline);
        thePainter.drawLine(topLine);
        
        // Draw the bottom line of the bottom highlight
        QLine bottomLine = QLine(QPoint(0, frame.bottom()), 
                              QPoint(frame.right(), frame.bottom()));
        thePainter.setPen(palette().light().color());
        thePainter.drawLine(bottomLine);
    
        // Fill in the bottom right corner with a single dark pixel
        bottomLine = QLine(QPoint(frame.left(), frame.bottom()), 
                           QPoint(frame.left(), frame.bottom()));
        thePainter.setPen(darkOutline);
        thePainter.drawLine(bottomLine);    
    
        // Fill in the bottom left corner with a single dark pixel
        bottomLine = QLine(QPoint(frame.right(), frame.bottom()), 
                           QPoint(frame.right(), frame.bottom()));
        thePainter.setPen(darkOutline);
        thePainter.drawLine(bottomLine);    
    }
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::initPanelsMenu()
//
/// Set up the panels action menu
//-----------------------------------------------------------------------------

// TODO gragan: These values should be in our studio style.
static const int kMaxWidth = 40;
static const int kMaxHeight = 18;
static const int kMenuFontSize = kFontSize;

void
WorkspacePanelGroup::initPanelsMenu()
{
	// Create the panels menu
	mPanelsMenu = new QToolButton(this);
	mPanelsMenu->setAutoRaise(true);
	mPanelsMenu->setFocusPolicy(Qt::NoFocus);
	mPanelsMenu->resize(kMaxWidth, kMaxHeight);
    mPanelsMenu->setPopupMode(QToolButton::InstantPopup);
    mPanelsMenu->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QIcon menuIcon;
    menuIcon.addPixmap(QString::fromUtf8(":/themes/normal/icons/workspace-normal.png"), QIcon::Normal, QIcon::On);
    menuIcon.addPixmap(QString::fromUtf8(":/themes/normal/icons/workspace-rollover.png"), QIcon::Active, QIcon::On);
    mPanelsMenu->setIcon(menuIcon);

	QSizePolicy actionPolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    actionPolicy.setHorizontalStretch(0);
    actionPolicy.setVerticalStretch(0);
    actionPolicy.setHeightForWidth(mPanelsMenu->sizePolicy().hasHeightForWidth());
    mPanelsMenu->setSizePolicy(actionPolicy);
    mPanelsMenu->setMinimumSize(QSize(kMaxWidth, kMaxHeight));
    mPanelsMenu->setMaximumSize(QSize(kMaxWidth, kMaxHeight));
            
    // Add the menu
    QMenu* menu = new QMenu(mPanelsMenu);
    mPanelsMenu->setMenu(menu);

    // Get signaled when the menu is about to show.
    QObject::connect(menu,
                     SIGNAL(aboutToShow()),
                     SLOT(onAboutToShowPanelsMenu()));

    // Use the system font face, but a smaller size.
    QFont menuFont = menu->font();
	menuFont.setPixelSize(kMenuFontSize);
    menu->setFont(menuFont);
    
    // Add the default menu items
    mClosePanelAction = menu->addAction("Close Panel");
    QObject::connect(mClosePanelAction, SIGNAL(triggered()), SLOT(onClosePanel()));

    mUndockPanelAction = menu->addAction("Undock Panel");
	QObject::connect(mUndockPanelAction, SIGNAL(triggered()), SLOT(onUndockCurrentPanel()));

    mUndockGroupAction = menu->addAction("Undock Group");
	QObject::connect(mUndockGroupAction, SIGNAL(triggered()), SLOT(onUndockGroup()));

	QAction* sep01 = menu->addAction("");
	sep01->setSeparator(true);

    // Add the New Panel hierarchical menu
    QAction* createPanelsAction = menu->addAction("New Panel");
    mCreatePanelsMenu = new QMenu(this);
    createPanelsAction->setMenu(mCreatePanelsMenu);
    mCreatePanelsMenu->addAction("Placeholder");
    mCreatePanelsMenu->setFont(menuFont);
    mCreatePanelsMenu->setEnabled(false);

	QAction* sep02 = menu->addAction("");
	sep02->setSeparator(true);

    // Add the Rename item
    QAction* renamePanelAction = menu->addAction("Rename Panel...");
    QObject::connect(renamePanelAction, 
                     SIGNAL(triggered()), 
                     this, 
                     SLOT(onTabRenameRequested()));
    
    // Add the menu to the tab bar
    // We need to create a container widget with a layout in it so we 
    // can position the menu properly in our tab bar area.
    QFrame* cornerFrame = new QFrame();
    cornerFrame->resize(tabBar()->rect().height(), tabBar()->rect().height()); 
    setCornerWidget(cornerFrame);

    // Create the layout to hold the menu and adjust the contents margins.
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 4, 2, 0);
    layout->setSpacing(0);
    cornerFrame->setLayout(layout);

    layout->addWidget(mPanelsMenu);
    
    // Add the dragger
    mDragWidget = new WorkspacePanelGroupDragWidget(this);
    layout->addWidget(mDragWidget);
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::onAboutToShowPanelsMenu()
// 
/// Populate the panels section of the panels menu
/// with the current set of panel creation actions.
//-----------------------------------------------------------------------------
void 
WorkspacePanelGroup::onAboutToShowPanelsMenu()
{
    // Set the active panel
    setActivePanel(currentIndex());

    // Get the workspace area that owns the menu
    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(parent());
    Q_ASSERT(workspaceArea != NULL);
        
    // Inform the area that is needs to populate the panel creation menu.
    workspaceArea->populateCreatePanelsMenu(mCreatePanelsMenu);
    
    // Get the layout
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(parentWidget()->layout());
    Q_ASSERT(theLayout != NULL);

    // Set the enabled states of the actions
    if (theLayout->count() == 1) {
        mClosePanelAction->setEnabled(false);    
        mUndockGroupAction->setEnabled(false);    
        mUndockPanelAction->setEnabled(count() > 1);
    } else {
        mUndockPanelAction->setEnabled(true);
        mUndockGroupAction->setEnabled(true);
    }        
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::onClosePanel()
//
/// Slot called to close active tab. Called using the action menu
/// on the tab bar.
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::onClosePanel()
{
    if (count() == 1) {       
        WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(parent());
        Q_ASSERT(workspaceArea != NULL);

        workspaceArea->removePanelGroup(this);
        delete this;
    } else {
        // We can just remove the tab.
        removeTab(currentIndex());
    }
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::onUndockCurrentPanel()
//
/// Slot called to undock the active tab. Called using the action menu
/// on the tab bar.
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::onUndockCurrentPanel()
{
    // Get the current item
    QWidget* widget = currentWidget();
    WorkspacePanel* panel = qobject_cast<WorkspacePanel*>(widget);
    Q_ASSERT(panel != NULL);

    // Undock the panel. We have to call setFloating twice so make 
    // sure we reparent and set the window topmost.  Otherwise,
    // QTabWidget will try to delete the widget.
    panel->setFloating(true);       
    removeTab(currentIndex());
    widget->setParent(parentWidget());
    panel->setFloating(true);   

    // We might have to update the layout if we floated the last 
    // panel in the tab group.
    if (count() == 0) {
        WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(parentWidget()->layout());
        Q_ASSERT(theLayout != NULL);
        theLayout->removePanelGroup(this);
        delete this;
    }        
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::onUndockGroup()
//
/// Slot called to undock the panel group. Called using the action menu
/// on the tab bar.
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::onUndockGroup()
{
    // Undock the panel. We have to call setFloating twice so make 
    // sure we reparent and set the window topmost.  Otherwise,
    // QTabWidget will try to delete the widget.
    setWindowFlags(Qt::Window);
    QMetaObject::invokeMethod(this, "show", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "raise", Qt::QueuedConnection);
    setWindowTitle("Panel Group");
    
    // Update the layout
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(parentWidget()->layout());
    Q_ASSERT(theLayout != NULL);
    theLayout->removePanelGroup(this);
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::onTabRenameRequested()
//
/// Slot called to rename the active tab. Called using the action menu
/// on the tab bar.
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::onTabRenameRequested()
{
    Q_EMIT tabRenameRequested(tabBar());
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::onTabChanged()
//
/// Slot called to rename the active tab. Called using the action menu
/// on the tab bar.
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::onTabChanged(int inIndex)
{
    setActivePanel(inIndex);
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::setActivePanel()
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::setActivePanel(int inIndex)
{
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::setActive()
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::setActive(bool inActive)
{
    if (inActive == mActive)
        return;

    mActive = inActive;        
}


//-----------------------------------------------------------------------------
// WorkspacePanelGroup::setConstraints()
//-----------------------------------------------------------------------------
void
WorkspacePanelGroup::setConstraints(int inX, 
                                    int inY, 
                                    int inWidth, 
                                    int inHeight)
{
    // Get the layout
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(parentWidget()->layout());
    Q_ASSERT(theLayout != NULL);

    // Get the constraints
    DynamicGridConstraints& constraints = theLayout->getConstraints(this);                    
    constraints.setX(inX);
    constraints.setY(inY);
    constraints.setWidth(inWidth);
    constraints.setHeight(inHeight);
}



