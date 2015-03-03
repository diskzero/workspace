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

#ifndef WORKSPACEAREA_HAS_BEEN_INCLUDED
#define WORKSPACEAREA_HAS_BEEN_INCLUDED

// Qt
#include <QColor>
#include <QRegion>
#include <QWidget>
#include <QXmlStreamReader>

// Forward declarations
class QIODevice;
class QMenu;
class QTabWidget;
class QXmlStreamReader;
class WorkspacePanelGroup;
class WorkspacePanel;

struct FloatingPanelPlaceHolder
{
    QString title;
    QRect geometry;
    bool visible;
    
    bool operator==(const FloatingPanelPlaceHolder& other) const
    {
        if (other.title == title)
            return true;
        return false;
    }
};

/// \brief A widget that can be used as the central area of a QMainWindow.
///
/// It provides docking panel functionality.
///
class WorkspaceArea : public QWidget
{
    Q_OBJECT

public:

    struct SavedGroup 
    {
        QString name;
        int x;
        int y;
        int width;
        int height;
        int gridx;
        int gridy;
        int gridwidth;
        int gridheight;
        int active;
        QStringList panels;
    };

    typedef QMap<QString, SavedGroup> SavedGroups;
    typedef QList<WorkspacePanel*> PanelList;
    typedef QPair<QString, SavedGroups> SavedLayout;
    
    WorkspaceArea(QWidget* parent = 0);
    ~WorkspaceArea();

    void addPanel(WorkspacePanel* panel, Qt::Orientation orientation);

    void addFloatingPanel(WorkspacePanel* panel);

    void insertPanel(WorkspacePanel* position,
                     WorkspacePanel* panel,
                     Qt::Orientation orientation,
                     bool before = true);

    void addToPanelGroup(WorkspacePanelGroup* inGroup, WorkspacePanel* panel);
    void addToPanelGroup(WorkspacePanel* sibling, WorkspacePanel* panel);

    void removePanelGroup(WorkspacePanelGroup* inGroup);
    void removePanel(WorkspacePanel* inPanel);
    void removeFromFloatingPanelPlaceHolders(const QString& name);

    void closePanels();
    
    void beginDeferLayout();
    void endDeferLayout();
    
    void getPanelList(QList<WorkspacePanel*>& outList);
    WorkspacePanel* getActivePanel();
    void setActivePanel(WorkspacePanel* inPanel);
    void activateNextPanel(WorkspacePanel* inPanel);

    void saveState(QXmlStreamWriter& inStream, bool inStandAlone = false) const;
    void saveState(SavedLayout& outLayout) const;
    void saveState(QXmlStreamWriter& inStream, const SavedGroups& inGroups) const;

    bool restoreState(QXmlStreamReader& inStream, SavedLayout& outLayout);
    void restoreComplete();

    void populateCreatePanelsMenu(QMenu* inMenu);

    bool renamePanel(QTabWidget* inTabBar, const QString& inName);

    WorkspacePanel* getPanelAt(const QPoint& inPoint) const;

    const QString& getLayoutName() const;
    void setLayoutName(const QString& inName);
    
    static QColor ActivePanelColor;

Q_SIGNALS:
    void updateCreatePanelsMenu(QMenu* inMenu);
  
protected:
    // QWidget override
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent* paintEvent);

private:
    // No copying
    WorkspaceArea(const WorkspaceArea& c);
    WorkspaceArea& operator = (const WorkspaceArea& c);

    void init();

    struct DragState;

    void adjustCursor();

    bool findFloatingPanelPlaceHolder(const QString & name,
                                FloatingPanelPlaceHolder& outResult) const;

    QWidget* getPanelNearPoint(QPoint pos, int horzDx, int vertDx) const;
    
    void updateDragState(const QPoint& inPoint, DragState& inOutState) const;
    void updateDragState2(const QPoint& inPoint, DragState& inOutState) const;

    void updateSplitter(const QPoint& inPosition);

    QList<FloatingPanelPlaceHolder> mFloatingPanelPlaceHolders;
    DragState* mDragState;

    WorkspacePanelGroup* mActiveGroup;
    WorkspacePanel* mActivePanel;
    QString mLayoutName;
    
    friend QDebug operator << (QDebug dbg, const DragState& dragState);
};
inline const QString& WorkspaceArea::getLayoutName() const { return mLayoutName; }
inline void WorkspaceArea::setLayoutName(const QString& inName) { mLayoutName = inName; }


#endif // !WORKSPACEAREA_HAS_BEEN_INCLUDED

