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
#include "WorkspaceTabBar.h"

// Qt
#include <QtCore>
#include <QtGui>
#include <QApplication>
#include <QStyle>
#include <QStylePainter>
#include <QTabBar>

// Studio
//#include <widgets/ActionMenu.h>
//#include <widgets/SyncButton.h>

// Local
#include "WorkspaceArea.h"
#include "WorkspacePanel.h"
#include "WorkspacePanelGroup.h"

//=============================================================================
// class WorkspaceTabBar
//=============================================================================
static const int kFontSize = 11;


//-----------------------------------------------------------------------------
// WorkspaceTabBar::WorkspaceTabBar()
//-----------------------------------------------------------------------------
WorkspaceTabBar::WorkspaceTabBar(WorkspacePanelGroup* inParent)
    :   QTabBar(inParent)
    ,   mParent(inParent)
    ,   mHoverIndex(-1)
    ,   mDragging(false)
    ,   mActive(false)
{
    // Set the standard behavior
    setDrawBase(true);
    setElideMode(Qt::ElideRight);
	setExpanding(false);
	setMouseTracking(true);
    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(false);
    
    // Set the default font
	QFont smallFont = qApp->font();
	smallFont.setPixelSize(kFontSize);
    setFont(smallFont);
    
    // Watch for current tab changes.
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabCloseRequested(int)));

	// TODO gragan:  I would like to make the tabs smaller, but this
	// has a huge ripple effect across all of the theming code
	// to draw the tabs, position the tab icons, etc.
	//mTabBar->setStyleSheet("QTabBar::tab { height: 20px; }");
    
    // HACK: we must explicitly set the cursor, otherwise the cursor that
    // WorkspaceArea sets when hovering over a splitter will remain in
    // effect even when the mouse moves over us.
    // See: the Qt documentation for QWidget::setCursor
    setCursor(Qt::ArrowCursor);   
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::~WorkspaceTabBar()
//-----------------------------------------------------------------------------
WorkspaceTabBar::~WorkspaceTabBar() 
{
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::tabInserted()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::tabInserted(int inIndex)
{
	// Create the sync button
//	workspace::SyncButton* syncButton = new workspace::SyncButton();
//	syncButton->setCanCancel(false);
//	syncButton->setCanSync(true);
//	
//	// Add it to the tab
//	setTabButton(inIndex,
//			     QTabBar::LeftSide,
//			     syncButton);
//	syncButton->hide();
	
    // Hide the close button
    const ButtonPosition closeSide = 
        (ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 
                                           0, 
                                           this);
                                           
    QWidget* button = tabButton(inIndex, closeSide);
    if (button != NULL)
        button->setVisible(false);
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::tabRemoved()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::tabRemoved(int inIndex)
{
	Q_UNUSED(inIndex);
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::mousePressEvent()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::mousePressEvent(QMouseEvent* inEvent)
{
    // Make sure to call parent for tab dragging
   	QTabBar::mousePressEvent(inEvent);

#if 0
    // Activate the current tab.
    const int theTab = tabAt(inEvent->pos());
    if (theTab != -1)
        mParent->setActivePanel(theTab);
#endif
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::mouseMoveEvent()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::mouseMoveEvent(QMouseEvent* inEvent)
{
    // Update the mouse over tab and set close button visibility.
    if (inEvent->buttons() == Qt::NoButton) {
        const int hoverIndex = tabAt(inEvent->pos());
        
        if (hoverIndex != mHoverIndex) {

            // Hide the old close button            
            const ButtonPosition closeSide = (ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 
                                                                                0, 
                                                                                this);
            QWidget* button = tabButton(mHoverIndex, closeSide);
            if (button != NULL)
                button->setVisible(false);
                        
            // Update the current hover tab        
            mHoverIndex = hoverIndex;

            // Show the close button
            button = tabButton(mHoverIndex, closeSide);
            if (button != NULL)
				button->setVisible(true);                                
        }        
    } else {
        // Suppress drawing during a drag animation.
        mDragging = true;
    }
       
    // Make sure to call parent for tab dragging
    QTabBar::mouseMoveEvent(inEvent);
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::mouseReleaseEvent()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::mouseReleaseEvent(QMouseEvent* inEvent)
{
    // Make sure to call parent for tab dragging
   	QTabBar::mouseReleaseEvent(inEvent);

    // Give the possible tab drag animation time to finish
    // before allowing the active tab to draw.
   	startTimer(200);
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::mouseDoubleClickEvent()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::mouseDoubleClickEvent(QMouseEvent* inEvent)
{
    Q_UNUSED(inEvent);
    
    WorkspacePanelGroup* panelGroup = qobject_cast<WorkspacePanelGroup*>(parentWidget());
    Q_ASSERT(panelGroup != NULL);
    panelGroup->onUndockCurrentPanel();
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::leaveEvent()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::leaveEvent(QEvent* inEvent)
{
	Q_UNUSED(inEvent);
	
    // Update the mouse over tab and set close button visibility.
    if (mHoverIndex != -1) {

        // Hide the close button
        const ButtonPosition closeSide = 
            (ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 
                                               0, 
                                               this);
                                               
        QWidget* button = tabButton(mHoverIndex, closeSide);
        if (button != NULL)
            button->setVisible(false);
                            
        // Update the current hover tab        
        mHoverIndex = -1;
    }        
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::paintEvent()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::paintEvent(QPaintEvent* inEvent)
{
    // Paint the default control.
    QTabBar::paintEvent(inEvent);
        
    // Check and see if we need to do active panel painting.
    if (!mParent->isActive())
        return;

    // Paint the active indication.
    QStylePainter thePainter(this);
    thePainter.setPen(WorkspaceArea::ActivePanelColor);

    // Get the widget bounding rect.
    const QRect& frame = rect();

    if (mDragging) {
        // Draw the entire line, but don't include the tab.
        QLine theLine = QLine(QPoint(frame.left(), frame.bottom()), 
                              QPoint(frame.right(), frame.bottom()));
        thePainter.drawLine(theLine);    
    } else {
                
        // Get the active tab rect.
        const QRect tabFrame = tabRect(currentIndex());
        
        // Draw from the tab bar left side to the tab rect left.
        QLine theLine = QLine(QPoint(frame.left(), frame.bottom()), 
                              QPoint(tabFrame.left(), frame.bottom()));
        thePainter.drawLine(theLine);    

        // Draw the left side of the tab.
        theLine = QLine(tabFrame.bottomLeft(),
                        QPoint(tabFrame.left(), tabFrame.top() + 2)); 
        thePainter.drawLine(theLine);    

        // Draw the left side top corner point
        theLine = QLine(QPoint(tabFrame.left() + 1, tabFrame.top() + 1),
                        QPoint(tabFrame.left() + 1, tabFrame.top() + 1)); 
        thePainter.drawLine(theLine);    

        // Draw the top of the tab.
        theLine = QLine(QPoint(tabFrame.left() + 2, tabFrame.top()),
                        QPoint(tabFrame.right() - 2, tabFrame.top()));
        thePainter.drawLine(theLine);    

        // Draw the right side top corner point
        theLine = QLine(QPoint(tabFrame.right() - 1, tabFrame.top() + 1),
                        QPoint(tabFrame.right() - 1, tabFrame.top() + 1)); 
        thePainter.drawLine(theLine);    

        // Draw the right side of the tab.
        theLine = QLine(QPoint(tabFrame.right(), tabFrame.top() + 2),
                        tabFrame.bottomRight());
        thePainter.drawLine(theLine);    

        // Draw the tab right side to the tab bar side.
        theLine = QLine(QPoint(tabFrame.right(), frame.bottom()), 
                        QPoint(frame.right(), frame.bottom()));
        thePainter.drawLine(theLine);    
    }
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::timerEvent()
//-----------------------------------------------------------------------------
void 
WorkspaceTabBar::timerEvent(QTimerEvent* inEvent)
{
    // We hope that the tab drag is complete
    if (mDragging) {
        killTimer(inEvent->timerId());
        mDragging = false;
        update();
    }
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::onTabChanged()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::onTabChanged(int inIndex)
{
    setActivePanel(inIndex);
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::onTabCloseRequested()
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::onTabCloseRequested(int inIndex)
{
    WorkspacePanelGroup* panelGroup = qobject_cast<WorkspacePanelGroup*>(parentWidget());
    Q_ASSERT(panelGroup != NULL);
    panelGroup->onClosePanel();
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::setActivePanel()
//
/// Set the active panel.
/// \param inIndex The index of the panel to make active.
//-----------------------------------------------------------------------------
void 
WorkspaceTabBar::setActivePanel(int inIndex)
{
#if 0
    // Get the Workspace area
    WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(parent());
    Q_ASSERT(workspaceArea != NULL);
   
    // Set this panel group as the active panel group.
    // There must be at least one panel in the group
    const QVariant& theData = tabData(inIndex);
    QWidget* itemWidget = reinterpret_cast<QWidget*>(theData.value<quintptr>());
    if (itemWidget == NULL)
        return;
   
    // Get the panel
    WorkspacePanel* thePanel = qobject_cast<WorkspacePanel*>(itemWidget);
    Q_ASSERT(thePanel != NULL);

    // Set ourselves as the active tab bar    
    workspaceArea->setActivePanel(thePanel);
    setActive(true);
#endif
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::setActive()
//-----------------------------------------------------------------------------
void 
WorkspaceTabBar::setActive(bool inActive) 
{ 
    if (mActive == inActive)
        return;

    mActive = inActive; 
    update();
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::startProgress()
//
/// Activate the progress indicator
/// \param inIndex The tab to indicate progress on.
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::startProgress(int inIndex)
{
//	QWidget* theButton = tabButton(inIndex, QTabBar::LeftSide);
//	if (theButton == NULL)
//		return;
//
//	workspace::SyncButton* syncButton =
//		qobject_cast<workspace::SyncButton*>(theButton);
//	if (syncButton == NULL)
//		return;
//		
//	syncButton->show();
//	syncButton->start();				
}


//-----------------------------------------------------------------------------
// WorkspaceTabBar::stopProgress()
//
/// Deactivate the progress indicator
/// \param inIndex The tab to stop progress on.
//-----------------------------------------------------------------------------
void
WorkspaceTabBar::stopProgress(int inIndex)
{
//	QWidget* theButton = tabButton(inIndex, QTabBar::LeftSide);
//	if (theButton == NULL)
//		return;
//
//	workspace::SyncButton* syncButton =
//		qobject_cast<workspace::SyncButton*>(theButton);
//	if (syncButton == NULL)
//		return;
//		
//	syncButton->stop();
//	syncButton->hide();	
}


