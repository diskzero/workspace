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

#ifndef WORKSPACE_HAS_BEEN_INCLUDED
#define WORKSPACE_HAS_BEEN_INCLUDED

// Qt
#include <QDockWidget>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

// Local
#include "WorkspaceArea.h"

// Forward declarations
class QAction;
class QMainWindow;
class QMenu;
class QMenuBar;
class QSignalMapper;
class QTabWidget;
class QTimer;

class WorkspacePanel;

namespace workspace {
class WorkspaceItem;
}


namespace workspace {

// Defines
typedef QList<WorkspaceItem*> WorkspaceItemList;
typedef QList<WorkspacePanel*> WorkspacePanelList;
typedef QList<QAction*> PanelCreationActionList;

//=============================================================================
// struct SavedLayout
//=============================================================================
struct SavedLayout 
{
    SavedLayout(const QString& inName = "");
    
    void writeState(QXmlStreamWriter& inStream, WorkspaceArea* inArea);    

    bool operator==(const SavedLayout& other) const;

    QString name;
    QRect geometry;
    WorkspaceArea::SavedLayout layout;
};

//=============================================================================
// class Workspace
//=============================================================================
/*! Provides a container workspace window.
    The Workspace can handle the display and arrangement of
    central windows and dockable palettes.  The Workspace can
    be extended by the creation of custom WorkspaceItems.
*/
class Workspace : public QObject
{
    Q_OBJECT

public:
    Workspace(const QString& inTitle);
    virtual ~Workspace();

    void initialize();

    void addWorkspaceItem(WorkspaceItem* inItem,
                          const QString& inName);

    void addWidget(QWidget* inWidget,
                   QWidget* inGroup,
                   const QString& inName,
                   bool isFloating = false);

    void removeWorkspaceItem(WorkspaceItem* inItem);

    void beginDeferLayout();
    void endDeferLayout();

    void setPanelContentsMargins(int left, int top, int right, int bottom);

    void setDefaultStateFile(const QString&);
    void setUserStateFile(const QString&);

    QMainWindow* getMainWindow();
    QMenuBar* getMenuBar();

    QAction* getFileAction();
    QAction* getEditAction();
    QAction* getWindowsAction();
    QAction* getHelpAction();
#ifdef DEBUG
    QAction* getDebugAction();
#endif

    bool layoutExists(const QString& inLayoutName) const;   
    bool getSavedLayout(const QString& inLayoutName, SavedLayout& outLayout);
    void getGlobalLayout(SavedLayout& outLayout);
    const WorkspaceArea::SavedLayout& getCurrentLayout() const;

    const PanelCreationActionList& getCreationActionList() const;

    bool renamePanel(QTabWidget* inTabWidget, const QString& inName);
    WorkspacePanel* getWorkspacePanel(QWidget* inWidget) const;

    void setWindowTitle(const QString& inTitle);

    static const QString DefaultLayoutName;
                    
public Q_SLOTS:
    void restoreState();
    void restoreComplete();
    void saveState();
    virtual void onTabRenameRequested(QTabWidget* inTabBar);
    virtual void onUpdateCreatePanelsMenu(QMenu* inMenu);

protected:
    virtual void initMenus() = 0;
    void saveState(const QString&);
    bool switchLayout(const QString& inName);
    bool deleteLayout(const QString& inName);
    bool renameLayout(const QString& inName, const QString& inNewName);
    void getLayoutNames(QStringList& outNames);
    void closePanels();
    void addPanelCreationAction(QAction* inAction);
    void getOpenPanelsList(WorkspacePanelList& outList) const;

private Q_SLOTS:
    void setActiveSubWindow(QWidget* inWindow);
    void updateWindowsMenu();

private:
    static const int StateFileVersion = 3;
    static const int OldStateFileVersion = 2;
 
    bool eventFilter(QObject* inObject, QEvent* inEvent);

    void addItemMenus(WorkspaceItem* inItem);
    void mergeItemMenus(WorkspaceItem* inItem);
    void removeItemMenus(WorkspaceItem* inItem);
    void replaceItemMenus(WorkspaceItem* inItem);

    bool restoreState(QIODevice* inFile);

    void initializeDockInfo();
    void startSaveStateTimer();
    
    void updateSavedLayout(const QString& inLayoutName);

    typedef QMap<QString, SavedLayout> SavedLayoutMap;
    typedef QMapIterator<QString, SavedLayout> LayoutConstIterator;
    typedef QMutableMapIterator<QString, SavedLayout> LayoutIterator;

    QSignalMapper* mWindowMapper;

    QMenuBar* mMenuBar;
    QAction* mFileAction;
    QAction* mEditAction;
    QAction* mWindowsAction;
    QAction* mHelpAction;
#ifdef DEBUG
    QAction* mDebugAction;
#endif

    QMenu* mPanelsMenu;
        
    QString mTitle;
    QMainWindow* mMainWindow;
    
    QString mDefaultStateFile;
    QString mUserStateFile;
    QTimer* mSaveStateTimer;

    WorkspaceItemList mWorkspaceItems;
    int mPanelContentsMarginLeft;
    int mPanelContentsMarginTop;
    int mPanelContentsMarginRight;
    int mPanelContentsMarginBottom;
    bool mPanelContentsMarginsSet;

    int mLayoutDeferrals;

    SavedLayoutMap mLayouts;
    PanelCreationActionList mCreationActionList;

    WorkspaceArea::SavedLayout mSavedLayout;
};

inline QMainWindow* Workspace::getMainWindow() { return mMainWindow; }
inline QMenuBar* Workspace::getMenuBar() { return mMenuBar; }

inline QAction* Workspace::getFileAction() { return mFileAction; }
inline QAction* Workspace::getEditAction() { return mEditAction; }
inline QAction* Workspace::getWindowsAction() { return mWindowsAction; }
inline QAction* Workspace::getHelpAction() { return mHelpAction; }
#ifdef DEBUG
inline QAction* Workspace::getDebugAction() { return mDebugAction; }
#endif

inline const PanelCreationActionList& Workspace::getCreationActionList() const { return mCreationActionList; }
inline const WorkspaceArea::SavedLayout& Workspace::getCurrentLayout() const { return mSavedLayout; }

} // namespace workspace

#endif // WORKSPACE_HAS_BEEN_INCLUDED

