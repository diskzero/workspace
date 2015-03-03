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

#ifndef WORKSPACE_LAYOUT_HAS_BEEN_INCLUDED
#define WORKSPACE_LAYOUT_HAS_BEEN_INCLUDED

// Local
#include "DynamicGridLayout.h"

// Forward declarations
class TabBarDragger;
class WidgetAnimator;
class WorkspacePanel;
class WorkspacePanelGroup;
class WorkspacePanelDropIndicator;

//=============================================================================
// class WorkspaceLayout
//=============================================================================
class WorkspaceLayout : public DynamicGridLayout
{
    Q_OBJECT
    
public:
    typedef QList<QWidget*> WidgetList;

    explicit WorkspaceLayout(QWidget* inParent);
    virtual ~WorkspaceLayout();
    
    virtual void setGeometry(const QRect& inGeometry);

    void addPanel(WorkspacePanel* inPanel, Qt::Orientation inOrientation);
    void insertPanel(WorkspacePanel* inPosition,
                     WorkspacePanel* inPanel,
                     Qt::Orientation inOrientation,
                     bool inBefore);
    void insertPanel(WorkspacePanelGroup* inGroup,
                     WorkspacePanel* inPanel,
                     Qt::Orientation inOrientation,
                     bool inBefore);

    void addToPanelGroup(WorkspacePanelGroup* inGroup, WorkspacePanel* inPanel);
    void addToPanelGroup(WorkspacePanel* inSiblingPanel, WorkspacePanel* inPanel);

    void removePanel(WorkspacePanel* inPanel);
    void removePanelGroup(WorkspacePanelGroup* inGroup);

    void movePanels(const WidgetList& inPanels,
                    Qt::Orientations inOrientations,
                    const QPoint& inPosition);

    void hover(QWidget* inWidget, const QPoint& inPoint);    
    void endHover(QWidget* inWidget = NULL);
    
    void beginResizePanels(Qt::Orientations inOrientations, const WidgetList& inPanels);
    void resizePanels(Qt::Orientations inOrientations, const QPoint& inPosition);
    void endResizePanels();
    
    QPoint itemLocation(QWidget* inWidget);
    QSize itemSize(QWidget* inWidget);

    WorkspacePanelGroup* findPanelGroup(WorkspacePanel* inPanel) const;

private:
    void insertPanelGroup(WorkspacePanelGroup* inTargetGroup,
                          WorkspacePanelGroup* inInsertGroup,
                          Qt::Orientation inOrientation,
                          bool inBefore);

    void addWidgetAtLocation(QWidget* inWidget,
                             int inColumn,
                             int inRow,
                             Qt::Orientation inOrientation);

    void insertWidget(QWidget* inWidget,
                      int inColumn,
                      int inRow,
                      Qt::Orientation inOrientation);
    
    void getEdgeLeftItems(int inEdgePosition, WidgetList& outItems);
    void getEdgeRightItems(int inEdgePosition, WidgetList& outItems);
    void getEdgeTopItems(int inEdgePosition, WidgetList& outItems);
    void getEdgeBottomItems(int inEdgePosition, WidgetList& outItems);
    void removePanelGroupLater(WorkspacePanelGroup* inPanelGroup);
        
    WorkspacePanelDropIndicator* mDropIndicator; 
    TabBarDragger* mTabDragger;
    QWidget* mHoverPanel;
    WidgetList mResizeListOne;
    WidgetList mResizeListTwo;
    QPoint mLastResizePosition;
    int mSplitterSize;

};

#endif // !WORKSPACE_LAYOUT_HAS_BEEN_INCLUDED


