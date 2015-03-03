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

#ifndef DYNAMICGRIDLAYOUT_HAS_BEEN_INCLUDED
#define DYNAMICGRIDLAYOUT_HAS_BEEN_INCLUDED

// System
#include <iostream>
#include <string>

// Qt
#include <QLayout>
#include <QList>
#include <QMap>
#include <QWidget>

// Local
#include "WidgetAnimator.h"

//=============================================================================
// class Insets
//=============================================================================
class Insets
{

public:
    Insets();
    Insets(int inLeft, int inTop, int inRight, int inBottom);
    Insets(const Insets& inInsets);

    Insets& operator=(const Insets& inInsets);
    void copy(const Insets& inInsets);

    void set(int inLeft, int inTop, int inRight, int inBottom);

    int left() const;
    int top() const;
    int right() const;
    int bottom() const;    

private:
    int mLeft;
    int mTop;
    int mRight;
    int mBottom;
};

inline int Insets::left() const { return mLeft; }
inline int Insets::top() const { return mTop; }
inline int Insets::right() const { return mRight; }
inline int Insets::bottom() const { return mBottom; }



//=============================================================================
// class DynamicGridConstraints
//=============================================================================
class DynamicGridConstraints
{
public:
    enum ConstrainType {
        RELATIVE = -1,
        REMAINDER = 0,
        
        NONE = 0,
        BOTH,
        HORIZONTAL,
        VERTICAL,
        
        CENTER = 10,
        NORTH,
        NORTHEAST,
        EAST,
        SOUTHEAST,
        SOUTH,
        SOUTHWEST,
        WEST,
        NORTHWEST
    };
    
    DynamicGridConstraints();
    DynamicGridConstraints(const DynamicGridConstraints& inConstraints);

    DynamicGridConstraints& operator=(const DynamicGridConstraints& inConstraints);

    int x() const;
    void setX(int inX);

    int y() const;
    void setY(int inY);
    
    int left() const;
    int top() const;    
    int right() const;
    int bottom() const;

    int width() const;
    void setWidth(int inWidth);

    int height() const;
    void setHeight(int inHeight);

    int fill() const;
    void setFill(int inFill);

    float weightX() const;
    void setWeightX(float inWeight);

    float weightY() const;
    void setWeightY(float inWeight);
    
    const Insets& insets() const;
    void setInsets(const Insets& inInsets);
    
    QPoint location() const;
    QRect bounds() const;
    QSize size() const;

private:
    void copy(const DynamicGridConstraints& inConstraints);

    friend class DynamicGridLayout;    

    int mGridX;
    int mGridY;
    int mGridWidth;
    int mGridHeight;
    float mWeightX;
    float mWeightY;
    int mAnchor;
    int mFill;
    int mPadX; 
    int mPadY;
    int mMinWidth;
    int mMinHeight;
    Insets mInsets;
    int mTempX;
    int mTempY;
    int mTempWidth;
    int mTempHeight;    
};

inline int DynamicGridConstraints::x() const { return mGridX; }
inline int DynamicGridConstraints::y() const { return mGridY; }
inline int DynamicGridConstraints::left() const { return mGridX; }
inline int DynamicGridConstraints::top() const { return mGridY; }
inline int DynamicGridConstraints::right() const { return mGridX + mGridWidth; }
inline int DynamicGridConstraints::bottom() const { return mGridY + mGridHeight; }
inline int DynamicGridConstraints::width() const { return mGridWidth; }
inline int DynamicGridConstraints::height() const { return mGridHeight; }
inline const Insets& DynamicGridConstraints::insets() const { return mInsets; }
inline void DynamicGridConstraints::setInsets(const Insets& inInsets) { mInsets = inInsets; }
inline QPoint DynamicGridConstraints::location() const { return QPoint(mGridX, mGridY); }
inline float DynamicGridConstraints::weightX() const { return mWeightX; }
inline void DynamicGridConstraints::setWeightX(float inWeight) { mWeightX = inWeight; }
inline float DynamicGridConstraints::weightY() const { return mWeightY; }
inline void DynamicGridConstraints::setWeightY(float inWeight) { mWeightY = inWeight; }
inline int DynamicGridConstraints::fill() const { return mFill; }
inline void DynamicGridConstraints::setFill(int inFill) { mFill = inFill; }
inline QSize DynamicGridConstraints::size() const { return QSize(mGridWidth, mGridHeight); }



//=============================================================================
// class DynamicGridLayoutInfo
//=============================================================================
class DynamicGridLayoutInfo
{
public:
    DynamicGridLayoutInfo()
        :   width(0)
        ,   height(0)
        ,   startx(0)
        ,   starty(0)
    {
        static const int MAXGRIDSIZE = 128;

        mMinWidth = new int[MAXGRIDSIZE];
        memset(mMinWidth, 0, sizeof(int) * MAXGRIDSIZE);

        mMinHeight = new int[MAXGRIDSIZE];
        memset(mMinHeight, 0, sizeof(int) * MAXGRIDSIZE);

        mWeightX = new float[MAXGRIDSIZE];
        memset(mWeightX, 0, sizeof(int) * MAXGRIDSIZE);

        mWeightY = new float[MAXGRIDSIZE];
        memset(mWeightY, 0, sizeof(int) * MAXGRIDSIZE);
    }

    int width, height;          // number of cells horizontally, vertically
    int startx, starty;         // starting point for layout 
    int* mMinWidth;             // largest minimum width in each column 
    int* mMinHeight;             // largest minimum height in each row 
    float* mWeightX;            // largest weight in each column 
    float* mWeightY;            // largest weight in each row 
};


//=============================================================================
// class DynamicGridLayout
//=============================================================================
class DynamicGridLayout : public QLayout
{
    Q_OBJECT

public:
    typedef QMap<QWidget*, DynamicGridConstraints*> ConstraintsMap;    
    typedef QMapIterator<QWidget*, DynamicGridConstraints*> GridConstIterator;
    typedef QMutableMapIterator<QWidget*, DynamicGridConstraints*> GridIterator;

    DynamicGridLayout();
    DynamicGridLayout(const Insets& inInsets);
    virtual ~DynamicGridLayout();

    void addWidget(QWidget* inWidget, const DynamicGridConstraints& inConstraints);
    void removeWidget(QWidget* inWidget);

    // QLayout
    virtual void addItem(QLayoutItem* inItem);

    virtual QLayoutItem* itemAt(int inIndex) const;
    virtual QLayoutItem* takeAt(int inIndex);

    virtual int count() const;

    virtual QSize sizeHint() const;
    virtual QSize minimumSize() const;

    virtual void setGeometry(const QRect& inGeometry);

    void updateLayout();

    void setColumnWidth(int inColumn, int inWidth);
    void setRowHeight(int inRow, int inHeight);

    void setInsets(const Insets& inInsets);

    const ConstraintsMap& getConstraintsMap() const;
    DynamicGridConstraints& getConstraints(QWidget* inWidget);
    DynamicGridConstraints* getConstraints(int inX, int inY);

    void preferredLayoutSize(QWidget* inParent, QSize& outSize);
    void minimumLayoutSize(QWidget* inParent, QSize& outSize);
    
    void compressLayout(const QRect& inBounds);
    void fillEmptySpace(Qt::Orientation inOrientation = Qt::Horizontal);

    int columnCount();
    int rowCount();
    
    bool animate() const;
    void setAnimate(bool inAnimate);

    void beginDeferLayout();
    void endDeferLayout();
    bool deferLayout() const;
    
    void dumpLayout(const QString& inMessage = "");

protected:
    QPoint getLayoutOrigin() const;

    int** getLayoutDimensions();
    float** getLayoutWeights();

    void location(int x, int y, QPoint& outPoint);

    void adjustForGravity(DynamicGridConstraints* constraints, QRect& ioRect);

    QSize getMinSize(QWidget* inWidget, DynamicGridLayoutInfo* info);

    void layoutGrid(QWidget* parent);

    DynamicGridLayoutInfo* getGridInfo(QWidget* parent);        
    DynamicGridLayoutInfo* getLayoutInfo();

private:
    void setConstraints(QWidget* inWidget, const DynamicGridConstraints& inConstraints);
    
    DynamicGridConstraints defaultConstraints;
    DynamicGridLayoutInfo* mLayoutInfo;

    Insets mContainerInsets;

    QList<int> mColumnWidths;
    QList<int> mRowHeights;
    QList<float> mColumnWeights;
    QList<float> mRowWeights;

    ConstraintsMap mItems;

    WidgetAnimator mWidgetAnimator;
    bool mAnimate;
    bool mDeferLayout;
};

inline const DynamicGridLayout::ConstraintsMap& DynamicGridLayout::getConstraintsMap() const { return mItems; }
inline bool DynamicGridLayout::animate() const { return mAnimate; }
inline void DynamicGridLayout::setAnimate(bool inAnimate) { mAnimate = inAnimate; }
inline bool DynamicGridLayout::deferLayout() const { return mDeferLayout; }



#endif // DYNAMICGRIDLAYOUT_HAS_BEEN_INCLUDED

