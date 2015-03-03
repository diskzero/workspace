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
#include "WorkspaceLayout.h"

// Qt
#include <QApplication>
#include <QBitmap>
#include <QEvent>
#include <QMouseEvent>
#include <QtDebug>
#include <QTabBar>

// Local
#include "WorkspacePanel.h"
#include "WorkspacePanelDropIndicator.h"
#include "WorkspacePanelGroup.h"
#include "WorkspaceTabBar.h"
#include "WorkspaceUtils.h"


//=============================================================================
// class TabBarDragger
//=============================================================================
/// An event filter installed in a WorkspaceTabBar to handle dragging        		
// TODO mrequenes Once tab-dragging is fully operational, perhaps we can 
// nuke the drag-and-drop code from WorkspacePanel.

static const int kTabDragThreshold = 25;

class TabBarDragger : public QObject
{
public:
    TabBarDragger(WorkspaceLayout* inOwner)
    :	QObject(inOwner)
    ,	mOwner(inOwner)
    ,	mDragState(NULL)
    {   
        // QCursor is lately very finicky about the bitmaps you pass it.
        // They MUST be monochromatic. Convert them here just to be sure.
        QBitmap bm1(":/themes/normal/cursors/panel_move.bmp");
        QBitmap bm2(":/themes/normal/cursors/panel_move_mask.bmp");
        mDragCursor = QCursor(QBitmap::fromImage(bm1.toImage(),
        				      Qt::MonoOnly),
                              QBitmap::fromImage(bm2.toImage(),
                              Qt::MonoOnly),
                              0,
                              0);
    }

    ~TabBarDragger()
    {
        delete mDragState;
    }
    
    bool eventFilter(QObject* inObject, QEvent* inEvent);
    
private:
    void initDrag(const QPoint& pos, QWidget* inWidget);
    void startDrag(QWidget* inWidget);
    void endDrag(bool inCanceled = false);

    QCursor mDragCursor;

    WorkspaceLayout* mOwner;

    struct DragState {
        QPoint pressPos;
        bool dragging;
        QWidget* widget;
        QWidget* dragWidget;
    };
    
    DragState* mDragState;
};


//-----------------------------------------------------------------------------
// TabBarDragger::eventFilter()
//-----------------------------------------------------------------------------
bool 
TabBarDragger::eventFilter(QObject* watched, QEvent* event)
{
    // Perform an initial filter for the events we handle.
    switch (event->type()) {
	    case QEvent::KeyPress:
	    case QEvent::MouseButtonDblClick:
	    case QEvent::MouseButtonPress:
	    case QEvent::MouseButtonRelease:
	    case QEvent::MouseMove:
	        break;
	        
	    default:
	        return false;
    }

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    QWidget* dragWidget = qobject_cast<QWidget*>(watched);

    // This could be a tab bar drag.
    QTabBar* tabBar = qobject_cast<QTabBar*>(watched);

    switch (event->type()) {
        	
		case QEvent::KeyPress:
		{	
	       if (mDragState != NULL) {
	           QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
	           if (keyEvent->key() == Qt::Key_Escape) {
	               // Escape key will cancel the drag
	               endDrag(true);
	               dragWidget->setCursor(Qt::ArrowCursor);
	               return true;
	           }
	       }
		}
		break;
	       	       
		case QEvent::MouseButtonPress:
		{	   
	       if (mouseEvent->button() == Qt::LeftButton) {
	            if (tabBar == NULL) {
	                // Drag the entire panel group
    	   			WorkspacePanelGroupDragWidget* theDragWidget = qobject_cast<WorkspacePanelGroupDragWidget*>(dragWidget);				
    	   			Q_ASSERT(theDragWidget != NULL);
    				initDrag(mouseEvent->pos(), theDragWidget->getPanelGroup());
	            } else {
	                // Drag the individual panel
	                const int index = tabBar->tabAt(mouseEvent->pos());            
    	   			Q_ASSERT(index != -1);
                    QWidget* parentWidget = dragWidget->parentWidget();
                    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(parentWidget);				
    	   			WorkspacePanel* thePanel = qobject_cast<WorkspacePanel*>(tabWidget->widget(index));				
    	   			Q_ASSERT(thePanel != NULL);
    				initDrag(mouseEvent->pos(), thePanel);
                }
                	           
	           // Don't consume event, or else tab clicking won't work.
	           return false; 
	       }
		}
		   
		case QEvent::MouseButtonRelease:
		{
			if (mDragState != NULL) {
				
				if (mDragState->dragging && mouseEvent->button() == Qt::LeftButton) {
					endDrag();
					dragWidget->setCursor(Qt::ArrowCursor);
				}
				
				delete mDragState;
				mDragState = NULL;

				// Don't consume the event. We want the WorkspaceTabBar to clean
				// up and handle tab drag positioning.
				return false;
			}
		}
		break;
	       
		case QEvent::MouseMove:
		{            
		    if (mDragState != NULL) {
			    // Check and see if we can break out of tab drag
			    if (!mDragState->dragging
			        && (mouseEvent->y() <= mDragState->pressPos.y() - kTabDragThreshold
			           || mouseEvent->y() >= mDragState->pressPos.y() + kTabDragThreshold)) {
			            		
			        // Try to begin a panel drag                                        
			        if ((mouseEvent->pos() - mDragState->pressPos).manhattanLength() > QApplication::startDragDistance()) {

                        // TODO gragan: We should snap the tab back to its original location.
                        // This is hard to figure out, as all the tab animation code
                        // is hidden inside of QTabBar.

                        // Begin the panel drag
                        startDrag(dragWidget);
			            
			            // Consume the event. We are doing all the mouse move handling ourself.
			            return true;
			        }                        
			    }
			
			    if (mDragState->dragging) {		        
			        // Begin hover drag
			        mOwner->hover(mDragState->widget, mouseEvent->globalPos());
			        return true;
			    }
			}					                
		    return false;
		}
		break;

    	default:
			break;    		
    }

    return false;
}


//-----------------------------------------------------------------------------
// TabBarDragger::initDrag()
//-----------------------------------------------------------------------------
void 
TabBarDragger::initDrag(const QPoint& inPosition, QWidget* inWidget)
{
    if (mDragState == NULL)
        mDragState = new TabBarDragger::DragState;
	
    mDragState->pressPos = inPosition;
    mDragState->dragging = false;
    mDragState->widget = inWidget;
    mDragState->dragWidget = NULL;
    
}


//-----------------------------------------------------------------------------
// TabBarDragger::startDrag()
//-----------------------------------------------------------------------------    
void 
TabBarDragger::startDrag(QWidget* inWidget)
{
    if (mDragState == NULL)     
        return;

    if (inWidget == NULL)
        return;

    mDragState->dragWidget = inWidget;
    mDragState->dragging = true;

    inWidget->setCursor(mDragCursor);
    inWidget->grabMouse();
    inWidget->grabKeyboard();
}


//-----------------------------------------------------------------------------
// TabBarDragger::endDrag()
//-----------------------------------------------------------------------------    
void
TabBarDragger::endDrag(bool inCanceled)
{
     Q_ASSERT(mDragState != NULL);
    
    if (inCanceled) {
        // Don't perform the drop move if the drag was canceled.
        mDragState->widget = NULL;
    }
    
    mDragState->dragWidget->releaseMouse();
    mDragState->dragWidget->releaseKeyboard();
    if (mDragState->dragging)
        mOwner->endHover(mDragState->widget);
    
    delete mDragState;
    mDragState = NULL;
        
    // TODO gragan: It would be nice to have some sort of
    // snap back animation if we have canceled the drag.
}


//=============================================================================
// class WorkspaceLayout
//
/// The workspace layout class provides a grid-based arrangement
/// for WorkspacePanels.  Every WorkspaceArea contains a WorkspaceLayout.
/// Items can added be added at a particular grid location and the item
/// will span horizontal and vertical space until either the layout edge
/// or another WorkspacePanel is encountered. It is not possible for the
/// layout to have an empty grid space.  All spaces must be occupied by
/// a WorkspacePanelGroup. When adding a WorkspacePanel, it will be
/// added to an existing group or a new group that is created at the
/// requested location.
//=============================================================================
static const int kSplitterSize = 5;
static const int kMinWidth = 150;
static const int kMinHeight = 250;

//-----------------------------------------------------------------------------
// WorkspaceLayout::WorkspaceLayout()
//-----------------------------------------------------------------------------
WorkspaceLayout::WorkspaceLayout(QWidget* inParent)
    :   DynamicGridLayout()
    ,   mDropIndicator(NULL)
    ,   mTabDragger(NULL)
    ,   mHoverPanel(NULL)
    ,   mSplitterSize(kSplitterSize)
{
    // Create and hide the drop zone indicator widget
    mDropIndicator = new WorkspacePanelDropIndicator(inParent);
    mDropIndicator->hide();

    // Set up the panel dragging
   	mTabDragger = new TabBarDragger(this);
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::~WorkspaceLayout()
//-----------------------------------------------------------------------------
WorkspaceLayout::~WorkspaceLayout()
{
    delete 	mTabDragger;
}     


//-----------------------------------------------------------------------------
// WorkspaceLayout::setGeometry()
//
/// Update the layout based on the new geometry passed in. This overrides
/// the parent class method to scale the layout rather than reapply the
/// updateLayout method.
/// \param inGeometry The new layout geometry.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::setGeometry(const QRect& inGeometry)
{
    if (inGeometry == geometry())
        return;

    // All a complete layout of all items the first time.
    DynamicGridLayout::setGeometry(inGeometry);
    
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::addPanel()
//
/// Add a panel at the specified orientation.
/// \param inPanel The panel to add.
/// \param inOrientation The orientation to add the panel at.
/// \sa Qt::Orientation
//-----------------------------------------------------------------------------
void
WorkspaceLayout::addPanel(WorkspacePanel* inPanel,
                          Qt::Orientation inOrientation)
{
    Q_ASSERT(inPanel != NULL);
    
    // Create the new group and add the panel to the group.
    WorkspacePanelGroup* group = new WorkspacePanelGroup();
    group->resize(kMinWidth, kMinHeight);
    group->addTab(inPanel, inPanel->objectName());

    // Hook up the event filter we use for panel dragging            
	group->getTabBar()->installEventFilter(mTabDragger);
	group->getDragWidget()->installEventFilter(mTabDragger);
        
    if (inOrientation == Qt::Vertical)  {
        // Add the item to the next empty row
        addWidgetAtLocation(group, 0, rowCount(), inOrientation);        
    } else {
        // Add the item to the next empty column
        addWidgetAtLocation(group, columnCount(), 0, inOrientation);       
    }
    
    if (!deferLayout()) {
        qDebug() << "addPanel";
        // Make sure any empty spaces get filled
        fillEmptySpace(Qt::Horizontal);
    }
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::insertPanel()
//
/// Insert the new panel into layout, either before or after the group
/// belonging to the panel passed in.
/// \param inPosition The panel to insert before or after.
/// \param inPanel The panel to insert into the layout.
/// \param The orientation to add the panel at.
/// \param True in the panel is to be inserted before the group.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::insertPanel(WorkspacePanel* inPosition,
                             WorkspacePanel* inPanel,
                             Qt::Orientation inOrientation,
                             bool inBefore)
{
    Q_ASSERT(inPosition != NULL);
    Q_ASSERT(inPanel != NULL);
    
    // Try to find the panel group containing the panel
    // passed in as a sibling.
     WorkspacePanelGroup* foundGroup = findPanelGroup(inPosition);
     if (foundGroup == NULL) {
        // We were unable to find the sibling.
        // Add the panel using the normal method.
        addPanel(inPanel, inOrientation);
    } else {        
        insertPanel(foundGroup,
                    inPanel,
                    inOrientation,
                    inBefore);
    }
}                           


//-----------------------------------------------------------------------------
// WorkspaceLayout::insertPanel()
//
/// Insert the new panel into layout, either before or after the group
/// belonging to the panel passed in.
/// \param inGroup The panel group to insert before or after.
/// \param inPanel The panel to insert into the layout.
/// \param The orientation to add the panel at.
/// \param True in the panel is to be inserted before the group.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::insertPanel(WorkspacePanelGroup* inGroup,
                             WorkspacePanel* inPanel,
                             Qt::Orientation inOrientation,
                             bool inBefore)
{
    Q_ASSERT(inGroup != NULL);
    Q_ASSERT(inPanel != NULL);
    
    // We will be inserting a new group no matter what.
    WorkspacePanelGroup* group = new WorkspacePanelGroup();
    group->setGeometry(inPanel->geometry());
    group->addTab(inPanel, inPanel->objectName());

    // Hook up the event filter we use for panel dragging            
	group->getTabBar()->installEventFilter(mTabDragger);

    // Get the location of the panel group that we found.
    QPoint location = itemLocation(inGroup);

    // Insert the widget at the requested location
    int theColumn = location.x();
    int theRow = location.y();

    if (inOrientation == Qt::Vertical)  {
        if (!inBefore)
            theRow++;
    } else {
        if (!inBefore)
            theColumn++;
    }
                    
    insertWidget(group, theColumn, theRow, inOrientation);
}                           


//-----------------------------------------------------------------------------
// WorkspaceLayout::insertPanelGroup()
//
/// Insert the new panel into layout, either before or after the group
/// belonging to the panel passed in.
/// \param inGroup The panel group to insert before or after.
/// \param inPanel The panel to insert into the layout.
/// \param The orientation to add the panel at.
/// \param True in the panel is to be inserted before the group.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::insertPanelGroup(WorkspacePanelGroup* inTargetGroup,
                                  WorkspacePanelGroup* inInsertGroup,
                                  Qt::Orientation inOrientation,
                                  bool inBefore)
{
    Q_ASSERT(inTargetGroup != NULL);
    Q_ASSERT(inInsertGroup != NULL);
    
    // Make sure the group being inserted has been removed from the layout.
    removeWidget(inInsertGroup);
    
    // Get the location of the target panel group
    QPoint location = itemLocation(inTargetGroup);

    // Insert the widget at the requested location
    int theColumn = location.x();
    int theRow = location.y();

    if (inOrientation == Qt::Vertical)  {
        if (!inBefore)
            theRow++;
    } else {
        if (!inBefore)
            theColumn++;
    }
                    
    insertWidget(inInsertGroup, theColumn, theRow, inOrientation);
}                           


//-----------------------------------------------------------------------------
// WorkspaceLayout::addToPanelGroup()
//
/// Add the panel to the group.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::addToPanelGroup(WorkspacePanelGroup* inGroup,
                                 WorkspacePanel* inPanel)
{
    Q_ASSERT(inGroup != NULL);
    Q_ASSERT(inPanel != NULL);
    
    Q_ASSERT(indexOf(inGroup) != -1);
    
    inGroup->addTab(inPanel, inPanel->objectName());
}

    
//-----------------------------------------------------------------------------
// WorkspaceLayout::addToPanelGroup()
//
/// Insert the new panel into the same group as the sibling panel passed in.
/// If the sibling cannot be found, the panel is added to a new location.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::addToPanelGroup(WorkspacePanel* inPosition,
                                 WorkspacePanel* inPanel)
{
    Q_ASSERT(inPosition != NULL);
    Q_ASSERT(inPanel != inPosition);
    
    // Try to find the panel group containing the panel
    // passed in as a sibling.
    WorkspacePanelGroup* foundGroup = findPanelGroup(inPosition);
    if (foundGroup == NULL) {
        // We were unable to find the sibling.            
        addPanel(inPanel, Qt::Horizontal);
    } else {
        // We found the sibling. Add the panel to same group.
        foundGroup->addTab(inPanel, inPanel->objectName());
    }
}                           


//-----------------------------------------------------------------------------
// WorkspaceLayout::addWidgetAtLocation()
//
/// Add the widget at the specified column and row.
/// \param inWidget The widget to add to the layout.
/// \param inColumn The column to add the widget at.
/// \param inRow The row to add the widget at.
/// \param inOrientation The orientation to add the widget at.
//-----------------------------------------------------------------------------
static const int kInsetAmount = 2;
static const float kDefaultWeight = 0.5;

void
WorkspaceLayout::addWidgetAtLocation(QWidget* inWidget,
                                     int inColumn,
                                     int inRow,
                                     Qt::Orientation inOrientation)
{
    Q_ASSERT(inWidget != NULL);
    
    Q_UNUSED(inOrientation);
     
    // Set up the default constraints
    DynamicGridConstraints theConstraints;
    theConstraints.setFill(DynamicGridConstraints::BOTH);
    theConstraints.setWeightX(kDefaultWeight);
    theConstraints.setWeightY(kDefaultWeight);
    theConstraints.setInsets(Insets(kInsetAmount,
                                    kInsetAmount,
                                    kInsetAmount,
                                    kInsetAmount));
    theConstraints.setX(inColumn);
    theConstraints.setY(inRow);
    
    // Add the widget to the layout using the constraints
    addWidget(inWidget, theConstraints);
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::insertWidget()
//
/// Insert the widget at the requested location.  The grid will expand
/// to accomodate the new location.
/// \param inWidget The widget to add.
/// \param inColumn The column to add the widget at.
/// \param inRow The row to add the widget at.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::insertWidget(QWidget* inWidget, 
                              int inColumn, 
                              int inRow,
                              Qt::Orientation inOrientation)
{
    Q_ASSERT(inWidget != NULL);
    Q_ASSERT(inColumn > -1);
    Q_ASSERT(inRow > -1);

    // Check to see if we already have this widget.
    if (getConstraintsMap().contains(inWidget))
        return;
        
    // Check and see if the requested location is occupied.
    DynamicGridConstraints* constraints = getConstraints(inColumn, inRow);
    if (constraints == NULL) {
        // The space in unoccupied. Simply add the new item.
        addWidgetAtLocation(inWidget, inColumn, inRow, inOrientation);
    } else {
        // The space is occupied.  Either a widget is anchored at the location or
        // it is spanning into the location.  Check to see if it is spanning
        // into the location. If so, we can adjust the span amount to make
        // room for the insertion.

        if (constraints->x() < inColumn) {
            // The item is being inserted.  Reduce the width of the found
            // item in order to create a column space for the new item.
            constraints->setWidth(constraints->width() - 1);
        } else if (inOrientation == Qt::Horizontal) {
            // Insert the new item into a new column and shift all other
            // items over a colum.
            GridConstIterator iter(getConstraintsMap());
            while (iter.hasNext()) {
                iter.next();       
                DynamicGridConstraints* nudgeItem = iter.value();
                Q_ASSERT(nudgeItem != NULL);                                
                if (nudgeItem->x() >= inColumn) {
                    nudgeItem->setX(nudgeItem->x() + 1);
                    nudgeItem->setWidth(1);
                }                    
            }
        }

        if (constraints->y() < inRow) {
            // The item is being inserted.  Reduce the height of the found
            // item in order to create a row space for the new item.
            constraints->setHeight(constraints->height() - 1);
            Q_ASSERT(constraints->height() >= 1);
        } else if (inOrientation == Qt::Vertical) {
            // Insert the new item into a new row and shift all other
            // items down a row.
            GridConstIterator iter(getConstraintsMap());
            while (iter.hasNext()) {
                iter.next();       
                DynamicGridConstraints* nudgeItem = iter.value();
                Q_ASSERT(nudgeItem != NULL);                                
                if (nudgeItem->y() >= inRow) {
                    nudgeItem->setY(nudgeItem->y() + 1);
                    nudgeItem->setHeight(1);
                }
            }
        }
    
        // We can add the widget now.
        addWidgetAtLocation(inWidget, inColumn, inRow, inOrientation);
    }

#if 1
    if (!deferLayout()) {
        qDebug() << "insertWidget";

        // Make sure any empty spaces get filled
        //fillEmptySpace(Qt::Horizontal);
        
        // Do a second pass to slide closed vertical slots            
        if (inOrientation == Qt::Vertical)
            fillEmptySpace(Qt::Vertical);
    }
#endif    
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::beginResizePanels()
//
/// Prepare for a panel resize operation.
/// Determine the panels that are going to be resized and populate
/// cached panel list.
/// \inOrientations The orientations of the resize drag.
/// \inPanels The primary reference drag panels.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::beginResizePanels(Qt::Orientations inOrientations,
                                   const WidgetList& inPanels)
{    
    // TODO mrequenes Handle moves of complex multi-panel intersections
    if (inPanels.size() != 2)
        return;
    
    // Reset the resize position
    mLastResizePosition = QPoint();
    
    // Reduce orientations down to a single orientation -- for now, at least.
    const Qt::Orientation orientation = inOrientations
                            == Qt::Vertical ? Qt::Vertical : Qt::Horizontal;
    
    // Get the sizes of the primary drag panels
    QRect firstRect = inPanels[0]->frameGeometry();
    QRect secondRect = inPanels[1]->frameGeometry();
    
    // +---------+---------+
    // |         |         |
    // |   A     |    C    |
    // |         |         |
    // |         +---------+
    // |         |         |
    // +---------+         |
    // |         |         |
    // |   B     |    D    |
    // |         |         |
    // +---------+---------+
    //
    // In the layout above, imagine the border between panels A and C
    // is being adjusted. Since C and D share an edge, D also has to
    // be adjusted, along with C.  We will need to locate these edge
    // items for each orientation and adjust them as well.
            
    if (orientation == Qt::Horizontal) {
        // Protect against out of order coordinates
        if (firstRect.left() >= secondRect.left())
            return;

        // Get the left and right edge items
        getEdgeLeftItems(firstRect.right(), mResizeListOne);
        getEdgeRightItems(secondRect.left(), mResizeListTwo);
    } else {
    
        // Protect against out of order coordinates
        if (firstRect.top() >= secondRect.top())
            return;

        // Get the top and bottom edge items
        getEdgeTopItems(firstRect.bottom(), mResizeListOne);
        getEdgeBottomItems(secondRect.top(), mResizeListTwo);
    }
}


//-----------------------------------------------------------------------------
// SortByWidthPredicate() [static]
//
// Sort the items in the list by width.
//-----------------------------------------------------------------------------
static bool
SortByWidthPredicate(const QWidget* inWidgetOne, const QWidget* inWidgetTwo)
{
    return inWidgetOne->width() < inWidgetTwo->width();
}


//-----------------------------------------------------------------------------
// SortByHeightPredicate() [static]
//
// Sort the items in the list by height.
//-----------------------------------------------------------------------------
static bool
SortByHeightPredicate(const QWidget* inWidgetOne, const QWidget* inWidgetTwo)
{
    return inWidgetOne->height() < inWidgetTwo->height();
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::resizePanels()
//-----------------------------------------------------------------------------
void
WorkspaceLayout::resizePanels(Qt::Orientations inOrientations,
                              const QPoint& inPosition)
{
    // Make sure there is work to do.
    if (mResizeListOne.isEmpty() || mResizeListTwo.isEmpty())
        return;

    // Reduce orientations down to a single orientation -- for now, at least.
    const Qt::Orientation orientation = inOrientations
            == Qt::Vertical ? Qt::Vertical : Qt::Horizontal;
        
    // Calculate the amount to be applied to the point passed in.
    // It is going to be some percentage of the splitter width, based
    // on the position of the mouse over it.
    // TODO mrequenes Would be better to use the actual offset between the mouse
    // and the center of the separator.
    const int halfSep = mSplitterSize / 2;
            
    if (orientation == Qt::Horizontal) {
        
        // Sort the items to be resized by width.
        std::sort(mResizeListOne.begin(), mResizeListOne.end(), SortByWidthPredicate);        
        std::sort(mResizeListTwo.begin(), mResizeListTwo.end(), SortByWidthPredicate);
        
        // Before we do anything, check and see if any of the widgets have
        // hit the size limit. If so, bail out.  We only check the items
        // on the other side of the resize position based on the resize direction.
        if (inPosition.x() < mLastResizePosition.x()) {
            Q_FOREACH(QWidget* leftItem, mResizeListOne) {
                if (leftItem->width() == kMinWidth)
                    return;
            }
        } else {
            Q_FOREACH(QWidget* rightItem, mResizeListTwo) {
                if (rightItem->width() == kMinWidth)
                    return;
            }
        }
                        
        // Resize the items on the left side of the resize point.
        // Make sure not to resize any item past the allowable limit.
        int rightLimit = 0;
        bool hitLimit = false;
        
        Q_FOREACH(QWidget* leftItem, mResizeListOne) {
            QRect leftRect = leftItem->geometry();
            if (hitLimit) {
                leftRect.setRight(rightLimit);        
            } else {
                leftRect.setRight(inPosition.x() - halfSep);        
                if (leftRect.width() < kMinWidth) {
                    hitLimit = true;
                    leftRect.setWidth(kMinWidth);
                    rightLimit = leftRect.right();
                }
            }
            leftItem->setGeometry(leftRect);
        }
    
        // Resize the items on the right side of the resize point.
        // Make sure not to resize any item past the allowable limit.
        QWidget* absoluteRightItem = mResizeListTwo[0];
        const int absoluteRight = absoluteRightItem->geometry().right();             
                
        int leftLimit = 0;
        if (hitLimit)
            leftLimit = rightLimit + mSplitterSize;
        
        Q_FOREACH(QWidget* rightItem, mResizeListTwo) {
            QRect rightRect = rightItem->geometry();
            if (hitLimit) {
                rightRect.setLeft(leftLimit);        
            } else {
                rightRect.setLeft(inPosition.x() + halfSep);        
                if (rightRect.width() < kMinWidth) {
                    hitLimit = true;
                    rightRect.setLeft(absoluteRight - kMinWidth + 1);
                    leftLimit = rightRect.left() + halfSep;
                }
            }
            rightItem->setGeometry(rightRect);
        }
        
        // TODO gragan: How can I avoid this?
        // Do a final cleaning pass on the left side item
        // to handle pixel encroachment that seems to happen
        // during really fast drags.
        const int absoluteLeft = absoluteRightItem->geometry().left();             
        Q_FOREACH(QWidget* leftItem, mResizeListOne) {
            QRect leftRect = leftItem->geometry();
            leftRect.setRight(absoluteLeft - mSplitterSize);        
            leftItem->setGeometry(leftRect);
        }
    } else {
        // Sort the items to be resized by height.
        std::sort(mResizeListOne.begin(), mResizeListOne.end(), SortByHeightPredicate);        
        std::sort(mResizeListTwo.begin(), mResizeListTwo.end(), SortByHeightPredicate);
        
        // Before we do anything, check and see if any of the widgets have
        // hit the size limit. If so, bail out.  We only check the items
        // on the other side of the resize position based on the resize direction.
        if (inPosition.y() < mLastResizePosition.y()) {
            Q_FOREACH(QWidget* topItem, mResizeListOne) {
                if (topItem->height() == kMinHeight)
                    return;
            }
        } else {
            Q_FOREACH(QWidget* bottomItem, mResizeListTwo) {
                if (bottomItem->height() == kMinHeight)
                    return;
            }
        }
                        
        // Resize the items on the top side of the resize point.
        // Make sure not to resize any item past the allowable limit.
        int bottomLimit = 0;
        bool hitLimit = false;
        
        Q_FOREACH(QWidget* topItem, mResizeListOne) {
            QRect topRect = topItem->geometry();
            if (hitLimit) {
                topRect.setBottom(bottomLimit);        
            } else {
                topRect.setBottom(inPosition.y() - halfSep);        
                if (topRect.height() < kMinHeight) {
                    hitLimit = true;
                    topRect.setHeight(kMinHeight);
                    bottomLimit = topRect.bottom();
                }
            }
            topItem->setGeometry(topRect);
        }
    
        // Resize the items on the bottom side of the resize point.
        // Make sure not to resize any item past the allowable limit.
        QWidget* absoluteBottomItem = mResizeListTwo[0];
        const int absoluteBottom = absoluteBottomItem->geometry().bottom();             
                
        int topLimit = 0;
        if (hitLimit)
            topLimit = bottomLimit + mSplitterSize;
        
        Q_FOREACH(QWidget* bottomItem, mResizeListTwo) {
            QRect bottomRect = bottomItem->geometry();
            if (hitLimit) {
                bottomRect.setTop(topLimit);        
            } else {
                bottomRect.setTop(inPosition.y() + halfSep);        
                if (bottomRect.height() < kMinHeight) {
                    hitLimit = true;
                    bottomRect.setTop(absoluteBottom - kMinHeight + 1);
                    topLimit = bottomRect.top() + halfSep;
                }
            }
            bottomItem->setGeometry(bottomRect);
        }

        // TODO gragan: How can I avoid this?
        // Do a final cleaning pass on the left side item
        // to handle pixel encroachment that seems to happen
        // during really fast drags.
        const int absoluteTop = absoluteBottomItem->geometry().top();             
        Q_FOREACH(QWidget* topItem, mResizeListOne) {
            QRect topRect = topItem->geometry();
            topRect.setBottom(absoluteTop - mSplitterSize);        
            topItem->setGeometry(topRect);
        }    
    }
    
    mLastResizePosition = inPosition;
}                            


//-----------------------------------------------------------------------------
// WorkspaceLayout::endResizePanels()
//
/// The panel resize is complete. Clean up any resize realated structures.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::endResizePanels()
{
    mResizeListOne.clear();
    mResizeListTwo.clear();
    mLastResizePosition = QPoint();
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::removePanelGroup()
//-----------------------------------------------------------------------------
void
WorkspaceLayout::removePanelGroup(WorkspacePanelGroup* inGroup)
{
    Q_ASSERT(inGroup != NULL);

    // We want these changes to be animated    
    setAnimate(true);
    removeWidget(inGroup);
    setAnimate(false);
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::itemLocation()
//
/// Return the item location in grid coordinates
/// \result The grid location
//-----------------------------------------------------------------------------
QPoint
WorkspaceLayout::itemLocation(QWidget* inWidget)
{
    Q_ASSERT(inWidget != NULL);
    
    const DynamicGridConstraints& constraints = getConstraints(inWidget);
    
    return constraints.location();
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::itemSize()
//
/// Return the item size in grid coordinates
/// \result The item size
//-----------------------------------------------------------------------------
QSize
WorkspaceLayout::itemSize(QWidget* inWidget)
{
    Q_ASSERT(inWidget != NULL);
    
    const DynamicGridConstraints& constraints = getConstraints(inWidget);
    
    return constraints.size();
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::hover()
// 
/// Provide feedback for a panel drag operation.
/// \param inPanel The panel that is hovering over the layout.
/// \param inPoint The location of the hover.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::hover(QWidget* inWidget, const QPoint& inPoint)
{
    const QWidget* parent = parentWidget();
    const QPoint localPos = parent->mapFromGlobal(inPoint);
    QWidget* wAtPos = parent->childAt(localPos);

    WorkspacePanelGroup* foundPanel = WorkspaceUtils::findParent<WorkspacePanelGroup>(wAtPos);
    if (foundPanel == NULL) {
        // TODO: Drag events are not being routed to the GraphicsView.
        // We need to manually trigger things right now.
        mDropIndicator->setDropPos(inPoint);
    } else {
        bool allowMove = false;
        if (foundPanel->currentWidget() == inWidget) {
            mDropIndicator->disableCentralArea();
            // Hovering over the same panel being "dragged". If the panel
            // doesn't belong to a tab group, do nothing, since it can't
            // be moved relative to itself.
            // Otherwise allow it, as the panel can be extracted from its
            // panel group and moved to another.
            allowMove = foundPanel->count() > 1;
        } else {
            allowMove = true;
            mDropIndicator->enableCentralArea();
        }
        
        if (allowMove) {
            mDropIndicator->setPanel(foundPanel);
            mDropIndicator->setDropPos(inPoint);
            mHoverPanel = foundPanel;
        } else {
            mDropIndicator->setPanel(NULL);
            mHoverPanel = NULL;
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::endHover()
//-----------------------------------------------------------------------------
void
WorkspaceLayout::endHover(QWidget* inWidget)
{    
    // Figure out what was passed in
    WorkspacePanel* inPanel = qobject_cast<WorkspacePanel*>(inWidget);
    WorkspacePanelGroup* inGroup = qobject_cast<WorkspacePanelGroup*>(inWidget);
    
    if (inPanel == NULL && inGroup == NULL) {
        mDropIndicator->setPanel(NULL);
        mHoverPanel = NULL;
        return;
    }

    if (mHoverPanel == NULL)
        return;
    
    // We want these changes to be animated
    setAnimate(true);
    
    // Get the panel group that we are hovering over    
    WorkspacePanelGroup* targetGroup = qobject_cast<WorkspacePanelGroup*>(mHoverPanel);
    if (targetGroup != NULL) {

        // Check and see if this is a floating panel.
        bool wasFloating = false;
        if (inPanel != NULL)
            wasFloating = inPanel->isFloating();

        // Find the panel group that the drag panel is part of.          
        WorkspacePanelGroup* sourcePanelGroup;
        if (wasFloating) {
            sourcePanelGroup = qobject_cast<WorkspacePanelGroup*>(inPanel->window());
        } else {
            if (inGroup != NULL) {
                // We are moving an entire panel group
                sourcePanelGroup = inGroup;
            } else {
                // In the single panel move case, we
                // will have to remove the tab from the source, create
                // a new panel group and add the the panel being moved
                // to the new panel group. It is possible for the 
                // sourcePanelGroup to be NULL if we are dealing with
                // a floating panel.
                sourcePanelGroup = findPanelGroup(inPanel);
            }
        }
        
        // Check and see if this is the last item in the group.
        // Also get the grid location of the panel group
        bool lastItem = false;
        QPoint panelOrigin;
        if (sourcePanelGroup != NULL && inGroup == NULL) {
            lastItem = sourcePanelGroup->count() == 1;

            // Get the current grid location of the item
            if (!wasFloating) {
                const DynamicGridConstraints& constraints = getConstraints(sourcePanelGroup);
                panelOrigin = constraints.location();
            }                
        }
        
        // Get the index of the item in the tab group.
        int panelIndex = -1;
        if (sourcePanelGroup != NULL && inPanel != NULL)
            panelIndex = sourcePanelGroup->indexOf(inPanel);

        // Get grid location of the target group.
        const QPoint gridPos = itemLocation(targetGroup);
                        
        bool changed = true;

        const WorkspacePanelDropIndicator::Area dropArea = mDropIndicator->getDropArea();
                
        // Perform the actions common to all drop zones
        switch (dropArea) {
	        case WorkspacePanelDropIndicator::AreaNorth:
	        case WorkspacePanelDropIndicator::AreaSouth:
 	        case WorkspacePanelDropIndicator::AreaWest:
	        case WorkspacePanelDropIndicator::AreaEast:
	        case WorkspacePanelDropIndicator::AreaCentral:
                {
                    // Remove the widget from the layout
                    if (lastItem)
                        removePanelGroupLater(sourcePanelGroup);
                    
                    if (sourcePanelGroup != NULL && inPanel != NULL) {
                        // Remove the panel tab from the group.
                        sourcePanelGroup->removeTab(panelIndex);
                        
                        // Activate the next item
                        sourcePanelGroup->setCurrentIndex(qMax(0, panelIndex - 1));
                        
                        // Set the position to the parent position. This will
                        // make the position change animation appear correct.
                        inPanel->move(sourcePanelGroup->pos());
                        
                        // The panel is now parentless until inserted below.
                        inPanel->setParent(NULL);
                    }
                }
                break;
                
            default:
                break;
        }                                                
                
        // Now determine where to do the insertion of the drop panel
        const bool BEFORE = true;
        const bool AFTER = false;
        
        switch (dropArea) {
	        case WorkspacePanelDropIndicator::AreaNone:
                changed = false;
	            break;
	                                        	            
	        case WorkspacePanelDropIndicator::AreaNorth:
	        {
	            if (inPanel != NULL)
                    insertPanel(targetGroup, inPanel, Qt::Vertical, BEFORE);
                else
                    insertPanelGroup(targetGroup, inGroup, Qt::Vertical, BEFORE);
            }
            break;
                	            
	        case WorkspacePanelDropIndicator::AreaSouth:
	        {
	            if (inPanel != NULL)
                    insertPanel(targetGroup, inPanel, Qt::Vertical, AFTER);
                else
                    insertPanelGroup(targetGroup, inGroup, Qt::Vertical, AFTER);
            }
            break;
                	            
	        case WorkspacePanelDropIndicator::AreaWest:
	        {
	            if (inPanel != NULL)
                    insertPanel(targetGroup, inPanel, Qt::Horizontal, BEFORE);
                else
                    insertPanelGroup(targetGroup, inGroup, Qt::Horizontal, BEFORE);
            }
            break;

	        case WorkspacePanelDropIndicator::AreaEast:
	        {
	            if (inPanel != NULL)
                    insertPanel(targetGroup, inPanel, Qt::Horizontal, AFTER);
                else
                    insertPanelGroup(targetGroup, inGroup, Qt::Horizontal, AFTER);
            }
            break;
	            
	        case WorkspacePanelDropIndicator::AreaCentral:
                {                    
                    if (inGroup == NULL) {
                        // Add the panel into the layout and make the panel
                        // the current active tab.
                        targetGroup->addTab(inPanel, inPanel->objectName());
                        targetGroup->setCurrentIndex(targetGroup->indexOf(inPanel));
                    } else {
                        // Move all of the items into the target group.
                        while (inGroup->count() > 0) {
                            QWidget* panel = inGroup->widget(0);
                            inGroup->removeTab(0);
                            targetGroup->addTab(panel, panel->objectName());
                        }
                        
                        // Remove the empty panel
                        removePanelGroupLater(sourcePanelGroup);
                    }
                }	            
                break;
			
			default:
				break;
        }
        
        // Reset the drop indicator
        mDropIndicator->setPanel(NULL);
        mHoverPanel = NULL;

        if (changed) { 

            // Hide the drop indicator before animating
            mDropIndicator->hide();
                        
            // Make sure the layout is updated
            updateLayout();
        }
        
        // Turn off animation
        setAnimate(false);
    }
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::findPanelGroup()
//
/// Find the group containing the panel.
/// \param inPanel The panel to look for.
/// \result The group, or NULL if none is found.
//-----------------------------------------------------------------------------
WorkspacePanelGroup*
WorkspaceLayout::findPanelGroup(WorkspacePanel* inPanel) const
{
    GridConstIterator iter(getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        if (theGroup != NULL) {
            for (int child = 0; child < theGroup->count(); child++) {
                if (theGroup->widget(child) == inPanel)
                    return theGroup;
            }
        }
    }
        
    return NULL;
}

//-----------------------------------------------------------------------------
// edgeCompare()
//
/// Perform a fuzzy compare for edge threshold.
//-----------------------------------------------------------------------------
static const int kEdgeSlop = 2;

static bool
edgeCompare(int inEdge, int inPosition)
{
    const int leftEdge = inEdge - kEdgeSlop;
    const int rightEdge = inEdge + kEdgeSlop;
    
    return inPosition >= leftEdge && inPosition <= rightEdge;
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::getEdgeLeftItems()
//
/// Get all items that are left of the edge.
/// \param inEdgePosition The edge position coordinate.
/// \result A list of items that are left of and share the edge.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::getEdgeLeftItems(int inEdgePosition, WidgetList& outItems)
{
    outItems.clear();

    GridConstIterator iter(getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        if (theGroup != NULL) {
            const QRect& frame = theGroup->frameGeometry();
            if (edgeCompare(frame.right(), inEdgePosition))
                outItems.push_back(theGroup);
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::getEdgeRightItems()
//
/// Get all items that are right of the edge.
/// \param inEdgePosition The edge position coordinate.
/// \result A list of items that are right of and share the edge.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::getEdgeRightItems(int inEdgePosition, WidgetList& outItems)
{
    outItems.clear();
    
    GridConstIterator iter(getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        if (theGroup != NULL) {
            const QRect& frame = theGroup->frameGeometry();
            if (edgeCompare(frame.left(), inEdgePosition))
                outItems.push_back(theGroup);
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::getEdgeTopItems()
//
/// Get all items that are above the edge.
/// \param inEdgePosition The edge position coordinate.
/// \result A list of items that are above and share the edge.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::getEdgeTopItems(int inEdgePosition, WidgetList& outItems)
{
    outItems.clear();

    GridConstIterator iter(getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        if (theGroup != NULL) {
            const QRect& frame = theGroup->frameGeometry();
            if (edgeCompare(frame.bottom(), inEdgePosition))
                outItems.push_back(theGroup);
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::getEdgeBottomItems()
//
/// Get all items that are below the edge.
/// \param inEdgePosition The edge position coordinate.
/// \result A list of items that are below and share the edge.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::getEdgeBottomItems(int inEdgePosition, WidgetList& outItems)
{
    outItems.clear();
    
    GridConstIterator iter(getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        if (theGroup != NULL) {
            const QRect& frame = theGroup->frameGeometry();
                            
            if (edgeCompare(frame.top(), inEdgePosition))
                outItems.push_back(theGroup);
        }
    }
}



//-----------------------------------------------------------------------------
// WorkspaceLayout::removePanel()
// 
/// Remove the panel group from the layout and mark it for deferred deletion.
/// We delete it later as the tab drag watcher may still be holding
/// onto a reference until the panel drag is complete.
/// \param inPanelGroup The panel group to remove and delete.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::removePanel(WorkspacePanel* inPanel)
{
    Q_ASSERT(inPanel != NULL);

    // Find the panel group for this item
    WorkspacePanelGroup* panelGroup = findPanelGroup(inPanel);
    Q_ASSERT(panelGroup != NULL);

    // Get the index of the widget
    const int tabIndex = panelGroup->indexOf(inPanel);
    Q_ASSERT(tabIndex != -1);

    // Remove the item
    panelGroup->removeTab(tabIndex);

    // We might have to remove the panel group as well
    if (panelGroup->count() == 0)
        removePanelGroupLater(panelGroup);
}


//-----------------------------------------------------------------------------
// WorkspaceLayout::removePanelGroupLater()
// 
/// Remove the panel group from the layout and mark it for deferred deletion.
/// We delete it later as the tab drag watcher may still be holding
/// onto a reference until the panel drag is complete.
/// \param inPanelGroup The panel group to remove and delete.
//-----------------------------------------------------------------------------
void
WorkspaceLayout::removePanelGroupLater(WorkspacePanelGroup* inPanelGroup)
{
    Q_ASSERT(inPanelGroup != NULL);

    removePanelGroup(inPanelGroup);
    inPanelGroup->hide();
    inPanelGroup->deleteLater();
}

