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

#ifndef LayoutEngine_HAS_BEEN_INCLUDED
#define LayoutEngine_HAS_BEEN_INCLUDED

#include <QLayoutItem>

template <typename T> class QVector;

// Mostly copied (with permission) from Qt's QLayoutStruct class in
// qlayoutengine_p.h
struct LayoutStruct
{
    inline void init(int stretchFactor = 0, int minSize = 0) {
        stretch = stretchFactor;
        minimumSize = sizeHint = minSize;
        maximumSize = QLAYOUTSIZE_MAX;
        expansive = false;
        empty = true;
        spacing = 0;
    }

    int smartSizeHint() {
        return (stretch > 0) ? minimumSize : sizeHint;
    }
    int effectiveSpacer(int uniformSpacer) const {
        Q_ASSERT(uniformSpacer >= 0 || spacing >= 0);
        return (uniformSpacer >= 0) ? uniformSpacer : spacing;
    }

    // parameters
    int stretch;
    int sizeHint;
    int maximumSize;
    int minimumSize;
    bool expansive;
    bool empty;
    int spacing;

    // temporary storage
    bool done;

    // result
    int position;
    int size;
};


void qGeomCalc(QVector<LayoutStruct> &chain, int start, int count,
                            int pos, int space, int spacer = -1);

#endif // !LayoutEngine_HAS_BEEN_INCLUDED
