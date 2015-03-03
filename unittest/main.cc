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

#define QT_GUI_LIB 1 // This is needed to enable QWidget testing

// Studio
#include <logging_base/logging.h>

// Qt
#include <QApplication>

// Local
#include "TestWorkspace.h"

static int
runTest(QObject * obj, int argc, char* argv[]){
    int result = QTest::qExec(obj, argc, argv);
    delete obj;
    return result;
}

int
main(int argc, char *argv[])
{
    logging_base::configure(argc, argv);

    Q_INIT_RESOURCE(theme);

    QApplication app(argc, argv);

    int result = 0;

    result = runTest(new TestWorkspace(), argc, argv);

    return result;
}

