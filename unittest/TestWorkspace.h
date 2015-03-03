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

#ifndef TESTWORKSPACE_H_HAS_BEEN_INCLUDED
#define TESTWORKSPACE_H_HAS_BEEN_INCLUDED

// Local
#include "../Workspace.h"

// Qt
#define QT_GUI_LIB 1 // This is needed to enable QWidget testing
#include <QtGui> // This is needed to enable QWidget testing
#include <QtTest>

// Namespaces
using namespace workspace;

//=============================================================================
// class TestWorkspace
//=============================================================================
class TestWorkspace : public QObject
{
    Q_OBJECT
    
private Q_SLOTS:
    void testActions();
    void testWindows();
    void testWorkspaceItem();

};


#endif // TESTWORKSPACE_H_HAS_BEEN_INCLUDED

