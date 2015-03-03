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

#ifndef WORKSPACEPANEL_HAS_BEEN_INCLUDED
#define WORKSPACEPANEL_HAS_BEEN_INCLUDED

// Qt
#include <QFrame> 
#include <QLayout>
#include <QWidget>

// Forward declarations
class QMenu;
class QStyleOptionDockWidget;
class QToolButton;

//=============================================================================
// class WorkspacePanelLayout
//=============================================================================
class WorkspacePanelLayout : public QLayout
{
    Q_OBJECT

public:
    WorkspacePanelLayout(QWidget* parent = NULL);
    ~WorkspacePanelLayout();

    void addItem(QLayoutItem *item);
    QLayoutItem *itemAt(int index) const;
    QLayoutItem *takeAt(int index);
    int count() const;

    QSize maximumSize() const;
    QSize minimumSize() const;
    QSize sizeHint() const;

    void setGeometry(const QRect &r);

    QSize sizeFromContent(const QSize &content) const;

    enum Role { Content, CloseButton, TitleBar, RoleCount };
    QWidget* widgetForRole(Role r) const;
    void setWidgetForRole(Role r, QWidget* w);

    int minimumTitleWidth() const;
    int titleHeight() const;
    void updateMaxSize() const;
    
private:
    QVector<QLayoutItem*> mItemList;
};


//=============================================================================
// class WorkspacePanel
//=============================================================================
class WorkspacePanel : public QFrame
{
    Q_OBJECT

    Q_FLAGS(DockWidgetFeatures)
    Q_PROPERTY(bool floating READ isFloating WRITE setFloating)
    Q_PROPERTY(DockWidgetFeatures features READ features WRITE setFeatures NOTIFY featuresChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle DESIGNABLE true)


public:
    WorkspacePanel(const QString& title, QWidget* parent = NULL);
    ~WorkspacePanel();

    void setWidget(QWidget*);
    QWidget* widget() const;

    enum DockWidgetFeature {
        DockWidgetClosable    = 0x01,
        DockWidgetMovable     = 0x02,
        DockWidgetFloatable   = 0x04,

        DockWidgetFeatureMask = 0x07,
        NoDockWidgetFeatures  = 0x00,

        Reserved              = 0xff
    };
    Q_DECLARE_FLAGS(DockWidgetFeatures, DockWidgetFeature)

    void setFeatures(DockWidgetFeatures features);
    DockWidgetFeatures features() const;

    void setFloating(bool floating);
    bool isFloating() const;

    QAction* toggleViewAction() const;
    const QString& fixedWindowTitle() const;

    void setWindowState(bool floating);
    void titleBarDragEvent(QMoveEvent* inEvent);

    bool isAnimating() const;
    bool isOverWorkspace() const;

    bool hasFeature(WorkspacePanel::DockWidgetFeature inFeature) const;
    
    void startProgress();
    void stopProgress();
	
	bool isActive() const;
	void setActive(bool inActive);    

Q_SIGNALS:
    void dockLocationChanged();
    void featuresChanged(WorkspacePanel::DockWidgetFeatures features);
    void topLevelChanged(bool floating);
    void visibilityChanged(bool visible);

protected:
    void changeEvent(QEvent* event);
    bool event(QEvent* event);
    void paintEvent(QPaintEvent* paintEvent);
    void timerEvent(QTimerEvent* inEvent);
    void initStyleOption(QStyleOptionDockWidget* option) const;
     
private Q_SLOTS:
    void onSignal();
    void toggleView(bool);
    void toggleTopLevel();

private:
    Q_DISABLE_COPY(WorkspacePanel)

    friend class WorkspacePanelLayout;
    friend class WorkspaceAreaLayout;
    friend class WorkspaceAreaLayoutInfo;

    void init(const QString& inTitle = "");

	bool mActive;
    bool mHover;
    QCursor mDragCursor;
    WorkspacePanel::DockWidgetFeatures mFeatures;
    Qt::DockWidgetAreas mAllowedAreas;
    QAction* mToggleViewAction;
    int mTimerId;
    QRect mUndockedGeometry;
    QString mFixedWindowTitle;

    static int sPanelCount;
};

inline bool WorkspacePanel::hasFeature(WorkspacePanel::DockWidgetFeature inFeature) const
    { return (mFeatures & inFeature) == inFeature; }
inline bool WorkspacePanel::isAnimating() const { return false; }
inline WorkspacePanel::DockWidgetFeatures WorkspacePanel::features() const { return mFeatures; }
inline bool WorkspacePanel::isActive() const { return mActive; }


//=============================================================================
// class WorkspacePanelItem
//=============================================================================
class WorkspacePanelItem : public QWidgetItem
{
public:
    WorkspacePanelItem(WorkspacePanel* dockWidget);
    QSize minimumSize() const;
    QSize maximumSize() const;
    QSize sizeHint() const;

private:
    QLayoutItem* dockWidgetChildItem() const {
        if (WorkspacePanelLayout *layout = dockWidgetLayout())
            return layout->itemAt(0);;
        return 0;
    }

    WorkspacePanelLayout* dockWidgetLayout() const {
        QWidget *w = const_cast<WorkspacePanelItem*>(this)->widget();
        if (w != 0)
            return qobject_cast<WorkspacePanelLayout*>(w->layout());
        return 0;
    }
};

#endif // !WORKSPACEPANEL_HAS_BEEN_INCLUDED

