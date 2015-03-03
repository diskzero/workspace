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
#include "TestWorkspace.h"

// Local
#include "../WorkspaceItem.h"

class MyWorkspace : public workspace::Workspace
{
public:
    MyWorkspace() : Workspace("Test")
    {
    }

    virtual ~MyWorkspace()
    {

    }

private:
    virtual void initMenus()
    {

    }
};


class MyWorkspaceItem : public workspace::WorkspaceItem
{
public:
    MyWorkspaceItem(workspace::Workspace* inWorkspace) : workspace::WorkspaceItem(inWorkspace)
    {
    }

    virtual ~MyWorkspaceItem()
    {

    }

private:
};




void 
TestWorkspace::testActions()
{
    MyWorkspace workspace;
    workspace.initialize();
    
    QAction* fileAction = workspace.getFileAction();
    Q_ASSERT(fileAction != NULL);
    
    QAction* editAction = workspace.getEditAction();
    Q_ASSERT(editAction != NULL);
    
    QAction* windowsAction = workspace.getWindowsAction();
    Q_ASSERT(windowsAction != NULL);
    
    QAction* helpAction = workspace.getHelpAction(); 
    Q_ASSERT(helpAction != NULL);
}


void 
TestWorkspace::testWindows()
{
    MyWorkspace workspace;
    workspace.initialize();

    QMainWindow* window = workspace.getMainWindow();
    Q_ASSERT(window != NULL);

    QMenuBar* menu = workspace.getMenuBar();
    Q_ASSERT(menu != NULL);

}


void 
TestWorkspace::testWorkspaceItem()
{
    MyWorkspace workspace;
    workspace.initialize();

    MyWorkspaceItem* item = new MyWorkspaceItem(&workspace);
    workspace.addWorkspaceItem(item, "item");
    
    Workspace* itemBench = item->getworkspace();
    Q_ASSERT(itemBench != NULL);

    QAction* action = item->getAction();
    Q_ASSERT(action != NULL);

    QMenu* menu = item->addMenu("Test");
    Q_ASSERT(menu != NULL);

    const workspace::WorkspaceItem::MenuList& menuList = item->getMenuList();
    Q_ASSERT(menuList.size() > 0);

    item->setMenuMergeType(workspace::WorkspaceItem::ADD);
    workspace::WorkspaceItem::MenuMergeType mergeType = item->getMenuMergeType();
    Q_ASSERT(mergeType == workspace::WorkspaceItem::ADD);

    workspace.removeWorkspaceItem(item);
}

