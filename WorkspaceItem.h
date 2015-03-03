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

#ifndef WORKSPACEITEM_HAS_BEEN_INCLUDED
#define WORKSPACEITEM_HAS_BEEN_INCLUDED

// Qt
#include <QWidget>
#include <QtPlugin>

// Forward declarations
class QAction;
class QLayout;
class QMenu;

class WorkspacePanel;

namespace workspace {

// Forward declarations
class Workspace;

//=============================================================================
// class WorkspaceItem
//=============================================================================
/*! Provides a panel that can be added to a Workspace.
    Items can be created to host a central widget.  The item will
    be placed into the Workspace environment. A stock set of slots
    are provided.  Overriding these will allow better integration
    with the Workspace and facilite support of clipboard and
    selection features.
*/	
// TODO mrequenes Does this need to inherit QWidget?? It doesn't seem to 
// override any QWidget behavior. Probably better to switch to a HAS-A
// relationship (where the 'CentralWidget' is that thing had) instead of
// an IS-A (QWidget).
class WorkspaceItem : public QWidget
{
    Q_OBJECT
    
public:
    typedef QList<QMenu*> MenuList;
    
    enum MenuMergeType {
        NONE,
        ADD,
        MERGE,
        REMOVE,
        REPLACE        
    };
    
    explicit WorkspaceItem(Workspace* inWorkspace, QWidget* inParent = NULL, Qt::WindowFlags inFlags = Qt::Window);
    virtual ~WorkspaceItem();

    Workspace* getWorkspace() const;

    QAction* getAction() const;

    void setSaveSettingsOnClose(bool inSave);
    bool saveSettingsOnClose() const;

    virtual QRect geometryHint() const;

    void setMenuMergeType(MenuMergeType inMenuMergeType);
    MenuMergeType getMenuMergeType() const;
    
    const MenuList& getMenuList() const;
    QMenu* addMenu(const QString& inTitle);

    void setCentralWidget(QWidget*);
    QWidget* centralWidget() const;

    virtual Qt::DockWidgetArea getDockPlacementHint() const;

    // HACKY
    void setFloating(bool f) { mFloating = f; }
    bool floating() const { return mFloating; }

    WorkspacePanel* getOwningPanel() const;
    void setOwningPanel(WorkspacePanel* panel);

    /// Signal names to be used for method invocation
    static const char* Copy;
    static const char* Cut;
    static const char* Paste;
    static const char* Duplicate;               
    static const char* Delete;               
    static const char* SelectAll;               
    static const char* SelectNone;                       
    static const char* InvertSelection; 

    
Q_SIGNALS:
    void titleSet(const QString& inTitle);

public Q_SLOTS:
    void setTitle(const QString& inTitle);

    /// Override to support Workspace Copy requests.
    virtual void onCopy() {}

    /// Override to support Workspace Cut requests.
    virtual void onCut() {}

    /// Override to support Workspace Paste requests.
    virtual void onPaste() {}

    /// Override to support Workspace Duplicate requests.
    virtual void onDuplicate() {}

    /// Override to support Workspace Delete requests.
    virtual void onDelete() {}

    /// Override to support Workspace Select All requests.
    virtual void onSelectAll() {}

    /// Override to support Workspace Select None requests.
    virtual void onSelectNone() {}

    /// Override to support Workspace Invert Selection requests.
    virtual void onInvertSelection() {}              
    
private Q_SLOTS:
    void showMe(bool inShow);

protected:
    virtual void showEvent(QShowEvent* inEvent);
    virtual void hideEvent(QHideEvent* inEvent);
    virtual void changeEvent(QEvent* inEvent);
    virtual void closeEvent(QCloseEvent* inEvent);

private:
    Workspace* mWorkspace;
    QAction* mAction;
    bool mSaveSettings;
    bool mFloating;
    MenuMergeType mMenuMergeType;
    MenuList mMenuList;
    QLayout* mLayout;
    QWidget* mCentralWidget;
    WorkspacePanel* mOwningPanel;
};

inline WorkspaceItem::MenuMergeType WorkspaceItem::getMenuMergeType() const { return mMenuMergeType; }
inline const WorkspaceItem::MenuList& WorkspaceItem::getMenuList() const { return mMenuList; }


//=============================================================================
// class WorkspacePlugin
//=============================================================================
/*! Provides a plugin interface to the Workspace.
 *  You can use this mechanism to create plugins that will add
 *  user interface elements to the Workspace.
*/	
 class WorkspacePlugin : public QObject
{
    Q_OBJECT;
public:
    WorkspacePlugin();
    virtual ~WorkspacePlugin();

    /// Create a new Workspace item.
    virtual WorkspaceItem* newItem(Workspace* inWorkspace) = 0;

    /// Return a class name for the item, suitable for creating a menu option
    /// beneath a 'View' menu. Just return the name in title case. Do
    /// no prepend it with 'View' or 'Show'.
    virtual QString getClassName() const = 0;

protected Q_SLOTS:
    // called when item is destroyed
    void slotItemDestroyed(QObject* inObject);

protected:
    // connect item and insert into list
    void addItem(WorkspaceItem* inItem);

protected:
    QList<WorkspaceItem*> mItemList;
};

} // namespace workspace


// Declare the plugin interface.  This has to be done outside of the namesspace
// block, otherwise the Qt preprocessor will generate an error.
QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(workspace::WorkspacePlugin,"com.ElephantWarehouse.WorkspacePlugin/1.0")
QT_END_NAMESPACE


#endif // WORKSPACEITEM_HAS_BEEN_INCLUDED

