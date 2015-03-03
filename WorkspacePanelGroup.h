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

#ifndef WORKSPACE_PANEL_GROUP_HAS_BEEN_INCLUDED
#define WORKSPACE_PANEL_GROUP_HAS_BEEN_INCLUDED

// Qt
#include <QFrame> 
#include <QLayout>
#include <QTabWidget> 
#include <QWidget>

// Forward declarations
class QMenu;
class QStyleOptionDockWidget;
class QToolButton;
class WorkspacePanelGroup;
class WorkspaceTabBar;


//=============================================================================
// class WorkspacePanelGroupDragWidget
//=============================================================================
class WorkspacePanelGroupDragWidget : public QFrame
{
	Q_OBJECT
	 	
public:
   explicit WorkspacePanelGroupDragWidget(WorkspacePanelGroup* inParent);
   virtual ~WorkspacePanelGroupDragWidget();

    WorkspacePanelGroup* getPanelGroup() const;

Q_SIGNALS:
   void dragged();

protected:
	virtual void paintEvent(QPaintEvent* inEvent);
	
private:
    WorkspacePanelGroup* mPanelGroup;    
};
inline WorkspacePanelGroup* WorkspacePanelGroupDragWidget::getPanelGroup() const { return mPanelGroup; }


//=============================================================================
// class WorkspacePanelGroup
//=============================================================================
class WorkspacePanelGroup : public QTabWidget
{
    Q_OBJECT

public:
    WorkspacePanelGroup(QWidget* inParent = NULL);
    ~WorkspacePanelGroup();

    WorkspaceTabBar* getTabBar() const;
    WorkspacePanelGroupDragWidget* getDragWidget() const;

    bool isActive() const;
    void setActive(bool inActive);

    void setConstraints(int inX, int inY, int inWidth, int inHeight);
    
Q_SIGNALS:
    void tabRenameRequested(QTabBar* tabBar);

public Q_SLOTS:
    void onClosePanel();
    void onUndockCurrentPanel();
    void onUndockGroup();

protected:
    virtual void paintEvent(QPaintEvent* inEvent);
     
private Q_SLOTS:
    void onAboutToShowPanelsMenu();
    void onTabRenameRequested();
    void onTabChanged(int inIndex);

private:
    Q_DISABLE_COPY(WorkspacePanelGroup)

	void initPanelsMenu();
    void setActivePanel(int inIndex);

    WorkspaceTabBar* mTabBar;
	QToolButton* mPanelsMenu;
	QAction* mClosePanelAction;
	QAction* mUndockPanelAction;
	QAction* mUndockGroupAction;
	WorkspacePanelGroupDragWidget* mDragWidget;
	QMenu* mCreatePanelsMenu;
	bool mActive;
};
inline WorkspaceTabBar* WorkspacePanelGroup::getTabBar() const { return mTabBar; }
inline WorkspacePanelGroupDragWidget* WorkspacePanelGroup::getDragWidget() const { return mDragWidget; }
inline bool WorkspacePanelGroup::isActive() const { return mActive; }


#endif // !WORKSPACE_PANEL_GROUP_HAS_BEEN_INCLUDED


