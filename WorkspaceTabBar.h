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

#ifndef WORKSPACETABBAR_HAS_BEEN_INCLUDED
#define WORKSPACETABBAR_HAS_BEEN_INCLUDED

// Qt
#include <QTabBar>

// Forward declarations
class WorkspacePanelGroup;

//=============================================================================
// class WorkspaceTabBar
//=============================================================================
class WorkspaceTabBar : public QTabBar
{
    Q_OBJECT
    
public:
	explicit WorkspaceTabBar(WorkspacePanelGroup* inParent);
	virtual ~WorkspaceTabBar();

    void setActivePanel(int inIndex);
    void setActive(bool inActive);

    void startProgress(int inIndex);
    void stopProgress(int inIndex);

public Q_SLOTS:
    void onTabChanged(int inIndex);
    void onTabCloseRequested(int inIndex);

protected:
	virtual void tabInserted(int inIndex);
	virtual void tabRemoved(int inIndex);
    virtual void mousePressEvent(QMouseEvent* inEvent);
	virtual void mouseMoveEvent(QMouseEvent* inEvent);
    virtual void mouseReleaseEvent(QMouseEvent* inEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent* inEvent);
	virtual void leaveEvent(QEvent* inEvent);
	virtual void paintEvent(QPaintEvent* inEvent);
    virtual void timerEvent(QTimerEvent* inEvent);
    
private:
    WorkspacePanelGroup* mParent;
    int mHoverIndex;
	bool mDragging;
	bool mActive;
};


#endif // !WORKSPACETABBAR_HAS_BEEN_INCLUDED

