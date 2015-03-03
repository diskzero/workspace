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
#include "DynamicGridLayout.h"

// Qt
#include <QtDebug>


//=============================================================================
// class Insets
//=============================================================================

//-----------------------------------------------------------------------------
// Insets::Insets()
//
/// Create a layout Insets object.
/// \param inLeft The inset from the left
/// \param inTop The inset from the top
/// \param inRight The inset from the right
/// \param inBottom The inset from the bottom
//-----------------------------------------------------------------------------
Insets::Insets(int inLeft, int inTop, int inRight, int inBottom)
    :   mLeft(inLeft)
    ,   mTop(inTop)
    ,   mRight(inRight)
    ,   mBottom(inBottom)
{
}


//-----------------------------------------------------------------------------
// Insets::Insets()
//-----------------------------------------------------------------------------
Insets::Insets()
    :   mLeft(0)
    ,   mTop(0)
    ,   mRight(0)
    ,   mBottom(0)
{
}


//-----------------------------------------------------------------------------
// Insets::Insets()
//-----------------------------------------------------------------------------
Insets::Insets(const Insets& inInsets)
{
    copy(inInsets);
}


//-----------------------------------------------------------------------------
// Insets::opertor=()
//-----------------------------------------------------------------------------
Insets& Insets::operator=(const Insets& inInsets)
{
    copy(inInsets);

    return (*this);
}


//-----------------------------------------------------------------------------
// Insets::copy()
//-----------------------------------------------------------------------------
void
Insets::copy(const Insets& inInsets)
{
    mLeft = inInsets.mLeft;
    mTop = inInsets.mTop;
    mRight = inInsets.mRight;
    mBottom = inInsets.mBottom;
}


//-----------------------------------------------------------------------------
// Insets::set()
//
/// Set the inset values.
//-----------------------------------------------------------------------------
void 
Insets::set(int inLeft, int inTop, int inRight, int inBottom)
{
    mLeft = inLeft;
    mTop = inTop;
    mRight = inRight;
    mBottom = inBottom;
}


//=============================================================================
// class DynamicGridConstraints
//=============================================================================

//-----------------------------------------------------------------------------
// DynamicGridConstraints::DynamicGridConstraints()
//-----------------------------------------------------------------------------
DynamicGridConstraints::DynamicGridConstraints()
    :   mGridX(0)
    ,   mGridY(0)
    ,   mGridWidth(1)
    ,   mGridHeight(1)
    ,   mWeightX(0)
    ,   mWeightY(0)
    ,   mAnchor(CENTER)
    ,   mFill(NONE)
    ,   mPadX(0)
    ,   mPadY(0)
    ,   mTempX(0)
    ,   mTempY(0)
    ,   mTempWidth(0)
    ,   mTempHeight(0)
{

}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::DynamicGridConstraints()
//-----------------------------------------------------------------------------
DynamicGridConstraints::DynamicGridConstraints(const DynamicGridConstraints& inConstraints)
{
    copy(inConstraints);
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::operator=()
//-----------------------------------------------------------------------------
DynamicGridConstraints& 
DynamicGridConstraints::operator=(const DynamicGridConstraints& inConstraints)
{
    copy(inConstraints);

    return *this;
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::copy()
//-----------------------------------------------------------------------------
void 
DynamicGridConstraints::copy(const DynamicGridConstraints& inConstraints)
{
    mGridX = inConstraints.mGridX;
    mGridY = inConstraints.mGridY;
    mGridWidth = inConstraints.mGridWidth;
    mGridHeight = inConstraints.mGridHeight;
    mWeightX = inConstraints.mWeightX;
    mWeightY = inConstraints.mWeightY;
    mAnchor = inConstraints.mAnchor;
    mFill = inConstraints.mFill;
    mInsets = inConstraints.mInsets;
    mPadX = inConstraints.mPadX;
    mPadY = inConstraints.mPadY;
    mTempX = inConstraints.mTempX;
    mTempY = inConstraints.mTempY;
    mTempWidth = inConstraints.mTempWidth;
    mTempHeight = inConstraints.mTempHeight;
    mMinWidth = inConstraints.mMinWidth;
    mMinHeight = inConstraints.mMinHeight;
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::bounds()
//
/// Return the grid bounds of the constraints
/// \result The bounding rectangle.
//-----------------------------------------------------------------------------
QRect 
DynamicGridConstraints::bounds() const
{
    return QRect(left(), top(), width(), height());
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::setX()
//
/// Set the x grid position.
/// \param inX The x coordinate
//-----------------------------------------------------------------------------
void
DynamicGridConstraints::setX(int inX)
{
    mGridX = qMax(0, inX);
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::setY()
//
/// Set the y grid position.
/// \param inY The y coordinate
//-----------------------------------------------------------------------------
void
DynamicGridConstraints::setY(int inY)
{
    mGridY = qMax(0, inY);
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::setWidth()
//
/// Set the item width.
/// \param inWidth The new width
//-----------------------------------------------------------------------------
void
DynamicGridConstraints::setWidth(int inWidth)
{
    mGridWidth = qMax(1, inWidth);
}


//-----------------------------------------------------------------------------
// DynamicGridConstraints::setHeight()
//
/// Set the item height.
/// \param inHeight The new height
//-----------------------------------------------------------------------------
void
DynamicGridConstraints::setHeight(int inHeight)
{
    mGridHeight = qMax(1, inHeight);
}


//=============================================================================
// class DynamicGridLayout
//=============================================================================
static const int kDefaultSize = 100;
static const int kMaxGridSize = 128;

//-----------------------------------------------------------------------------
// DynamicGridLayout::DynamicGridLayout()
//
/// Create a DynamicGridLayout
//-----------------------------------------------------------------------------
DynamicGridLayout::DynamicGridLayout()
    :   mLayoutInfo(NULL)
    ,   mWidgetAnimator(this)
    ,   mAnimate(false)
    ,   mDeferLayout(false)

{
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::DynamicGridLayout()
//
/// Create a DynamicGridLayout
/// \param inInsets The layout insets.
//-----------------------------------------------------------------------------
DynamicGridLayout::DynamicGridLayout(const Insets& inInsets)
    :   mLayoutInfo(NULL)
    ,   mContainerInsets(inInsets)
    ,   mWidgetAnimator(this)
    ,   mAnimate(false)
    ,   mDeferLayout(false)
{
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::~DynamicGridLayout()
//-----------------------------------------------------------------------------
DynamicGridLayout::~DynamicGridLayout()
{
    Q_FOREACH(DynamicGridConstraints* constraints, mItems)
        delete constraints;
        
    delete mLayoutInfo;        
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::addWidget()
//
/// Add the widget using the constraints.
//-----------------------------------------------------------------------------
void
DynamicGridLayout::addWidget(QWidget* inWidget, 
                            const DynamicGridConstraints&  inConstraints)
{
    Q_ASSERT(inWidget != NULL);

    // Take ownership of this widget
    inWidget->setParent(parentWidget());

    // Set the layout constraint
    setConstraints(inWidget, inConstraints);

    if (!mDeferLayout) {
        // Update the layout
        updateLayout();
    
        // Close up any gaps that may have opened
        fillEmptySpace();
    }
    
    // Make sure the new item is visible
    inWidget->show();
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::removeWidget()
//
/// Remove the widget from the layout.
//-----------------------------------------------------------------------------
void
DynamicGridLayout::removeWidget(QWidget* inWidget)
{
    Q_ASSERT(inWidget != NULL);
    
    // Make sure the widget is in the layout
    if (!mItems.contains(inWidget))
        return;
    
    // Clear the constraints
    DynamicGridConstraints* constraints = mItems[inWidget];
    const QRect emptySpace = constraints->bounds();    
    delete constraints;

    // Remove the info from the item map.
    mItems.remove(inWidget);
        
    // Get rid of any empty grid locations
    compressLayout(emptySpace);
    
    // Close up any gaps that may have opened
    qDebug() << "removeWidget";
    fillEmptySpace();

    // Apply all of the changes
    updateLayout();
    
#if 0
    // The gaps in the layout have been closed, but we may still have
    // items that are encroaching past the maximum grid dimensions

    // Get the current grid width and height based on the item dimensions
    QMap<int, int> widthMap;
    QMap<int, int> heightMap;
    
    QMap<int, int> rowCountMap;
    QMap<int, int> columnCountMap;

    GridIterator iter(mItems);       
    while (iter.hasNext()) {
        iter.next();
        DynamicGridConstraints* constraint = iter.value();
        widthMap[constraint->y()] += constraint->width();
        columnCountMap[constraint->y()] += 1;

        heightMap[constraint->x()] += constraint->height();
        rowCountMap[constraint->x()] += 1;
    }
    
    QMapIterator<int, int> mapIter(widthMap);
    while (mapIter.hasNext()) {
        mapIter.next();
        qDebug() << mapIter.key() << mapIter.value();
    }
    
    // Find the row with the most columns. This
    // will become our reference row.
    int referenceRow = 0;
    int maxColumns = 0;
    QMapIterator<int, int> columnIter(columnCountMap);
    while (columnIter.hasNext()) {
        columnIter.next();
        
        if (columnIter.value() > maxColumns) {
            maxColumns = columnIter.value();
            referenceRow = columnIter.key();
        } 
    }
        
    // The widths of all the columns in the reference row
    // will be set to one column width.
    for (int colIndex = 0; colIndex < columnCountMap[referenceRow]; ++colIndex) {
        DynamicGridConstraints* constraints = getConstraints(colIndex, referenceRow);
        if (constraints != NULL)
            constraints->setWidth(1);
    }

    // Get the maximum allowable row width.
    const int maxWidth = columnCountMap[referenceRow];

    // Now conform the rest of the rows to the maximum width    
    for (int rowIndex = 0; rowIndex < rowCountMap.size(); ++rowIndex) {
        // Skip the reference row
        if (rowIndex != referenceRow) {
            
            // Get the width of the current row            
            int totalWidth = widthMap[rowIndex];
            
            // Start shrinking the item widths until we have a legal value.
            while (totalWidth > maxWidth) {
                for (int colIndex = 0; colIndex < maxWidth; ++colIndex) {                
                    // Try to shrink the row items
                    DynamicGridConstraints* constraints = getConstraints(colIndex, rowIndex);
                    if (constraints != NULL && constraints->width() > 1) {
                        constraints->setWidth(constraints->width() - 1);
                        totalWidth--;
                        qDebug() << "shrinking a column";
                    }
                }
            }
        }
    }
        
    // Apply all of the changes
    updateLayout();

    qDebug() << "done removing: " << columnCount();
#endif
}

    
//-----------------------------------------------------------------------------
// DynamicGridLayout::addItem()
//-----------------------------------------------------------------------------
void
DynamicGridLayout::addItem(QLayoutItem* inItem)
{
    Q_ASSERT(inItem != NULL);
    Q_UNUSED(inItem);
    Q_ASSERT(true);
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::itemAt()
//-----------------------------------------------------------------------------
QLayoutItem*
DynamicGridLayout::itemAt(int inIndex) const
{
    Q_UNUSED(inIndex);
    return NULL;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::takeAt()
//-----------------------------------------------------------------------------
QLayoutItem*
DynamicGridLayout::takeAt(int inIndex)
{
    Q_UNUSED(inIndex);
    return NULL;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::count()
//-----------------------------------------------------------------------------
int
DynamicGridLayout::count() const
{
    return mItems.size();
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::minimumSize()
//-----------------------------------------------------------------------------
QSize
DynamicGridLayout::minimumSize() const
{
    return QSize(kDefaultSize, kDefaultSize);
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::sizeHint()
//-----------------------------------------------------------------------------
QSize
DynamicGridLayout::sizeHint() const
{
    return QSize(kDefaultSize, kDefaultSize);
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::setGeometry()
//
/// Update the layout based on the new geometry passed in.
/// \param inGeometry The new layout geometry.
//-----------------------------------------------------------------------------
void
DynamicGridLayout::setGeometry(const QRect& inGeometry)
{
    if (inGeometry == geometry())
        return;

    const bool animating = mAnimate;
    mAnimate = false;
    QLayout::setGeometry(inGeometry);
    updateLayout();
    mAnimate = animating;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::setColumnWidth()
// 
/// Set the width of the specified column
/// \param inColumn The column to set the widht of.
/// \param inWidth The column widht
//-----------------------------------------------------------------------------
void
DynamicGridLayout::setColumnWidth(int inColumn, int inWidth)
{        
    // Don't allow out of bounds values
    if (inWidth < 1)
        return;
        
    if (inColumn < 0 || inColumn >= columnCount())
        return;
 
    updateLayout();
       
    mColumnWidths[inColumn] = inWidth;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::setRowHeight()
// 
/// Set the height of the specified row
/// \param inRow The row to set the height of.
/// \param inHeight The row height
//-----------------------------------------------------------------------------
void
DynamicGridLayout::setRowHeight(int inRow, int inHeight)
{    
    // Don't allow out of bounds values
    if (inHeight < 1)
        return;

    if (inRow < 0 || inRow >= rowCount())
        return;

    updateLayout();

    mRowHeights[inRow] = inHeight;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::setInsets()
// 
/// Change the layout insets
/// \param inInsets The new layout insets.
//-----------------------------------------------------------------------------
void
DynamicGridLayout::setInsets(const Insets& inInsets)
{
    mContainerInsets = inInsets;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::setConstraints()
// 
/// Set the layout constraints for the widget.
/// \param inWidget The widget to set the constraints for.
/// inConstraints The constraints.
//-----------------------------------------------------------------------------
void 
DynamicGridLayout::setConstraints(QWidget* inWidget, 
                                 const DynamicGridConstraints& inConstraints)
{
    Q_ASSERT(inWidget != NULL);
    Q_ASSERT(!mItems.contains(inWidget));

    mItems[inWidget] = new DynamicGridConstraints(inConstraints);
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getConstraints()
// 
/// Return the constraints for the widget.
/// \param inWidget The widget to get the constraints of.
/// \result The layout constraints for the widget.
//-----------------------------------------------------------------------------
DynamicGridConstraints&
DynamicGridLayout::getConstraints(QWidget* inWidget)
{
    Q_ASSERT(mItems.contains(inWidget));

    DynamicGridConstraints* constraints = mItems[inWidget];
    if (constraints == NULL) {
        setConstraints(inWidget, defaultConstraints);
        return getConstraints(inWidget);
    }

    return *constraints;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getLayoutOrigin()
// 
/// Get the layout origin
//-----------------------------------------------------------------------------
QPoint
DynamicGridLayout::getLayoutOrigin() const
{
    QPoint origin;

    if (mLayoutInfo != NULL) {
        origin.setX(mLayoutInfo->startx);
        origin.setY(mLayoutInfo->starty);
    }
    
    return origin;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getLayoutDimensions()
// 
/// Get the layout dimensions
//-----------------------------------------------------------------------------
int** 
DynamicGridLayout::getLayoutDimensions()
{
    if (mLayoutInfo == NULL)
        return NULL;

    int** dim = (int**)malloc(2 * sizeof(int*));

    dim[0] = new int[mLayoutInfo->width];
    dim[1] = new int[mLayoutInfo->height];

    memcpy(&mLayoutInfo->mMinWidth[0], &dim[0][0], mLayoutInfo->width);
    memcpy(&mLayoutInfo->mMinHeight[0], &dim[1][0], mLayoutInfo->height);

    return dim;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getLayoutWeights()
// 
/// Get the layout weights
//-----------------------------------------------------------------------------
float** 
DynamicGridLayout::getLayoutWeights()
{
    if (mLayoutInfo == NULL)
        return NULL;

    float** weights = (float**)malloc(2 * sizeof(float));
    weights[0] = new float[mLayoutInfo->width];
    weights[1] = new float[mLayoutInfo->height];

    memcpy(&mLayoutInfo->mWeightX[0], &weights[0][0], mLayoutInfo->width);
    memcpy(&mLayoutInfo->mWeightY[0], &weights[1][0], mLayoutInfo->height);

    return weights;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::location()
//-----------------------------------------------------------------------------
void 
DynamicGridLayout::location(int x, int y, QPoint& outPoint)
{
    outPoint = QPoint();

    if (mLayoutInfo == NULL)
        return;

    int index;        
    int d = mLayoutInfo->startx;
    for (index = 0; index < mLayoutInfo->width; ++index) {
        d += mLayoutInfo->mMinWidth[index];
        if (d > x)
            break;
    }
    outPoint.setX(index);

    d = mLayoutInfo->starty;
    for (index = 0; index < mLayoutInfo->height; ++index) {
        d += mLayoutInfo->mMinHeight[index];
        if (d > y)
            break;
    }
    outPoint.setY(index);
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::preferredLayoutSize()
// 
/// Returns the preferred dimensions for this layout given the widget
/// in the specific cell.
/// \param inParent The parent widget
/// \param outSize The dimension that will be retrieved.
/// \sa minimumLayoutSize
//-----------------------------------------------------------------------------
void 
DynamicGridLayout::preferredLayoutSize(QWidget* inParent, QSize& outSize)
{
    DynamicGridLayoutInfo* info = getLayoutInfo();
    outSize = getMinSize(inParent, info);

    delete info;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::minimumLayoutSize()
// 
/// Returns the minimum dimensions for this layout given the widget
/// in the specific cell.
/// \param inParent The parent widget
/// \param outSize The dimension that will be retrieved.
/// \sa preferredLayoutSize
//-----------------------------------------------------------------------------
void  
DynamicGridLayout::minimumLayoutSize(QWidget* inParent, QSize& outSize)
{
    DynamicGridLayoutInfo* info = getLayoutInfo();
    outSize = getMinSize(inParent, info);

    delete info;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::updateLayout()
// 
/// Perform the layout.
//-----------------------------------------------------------------------------
void 
DynamicGridLayout::updateLayout()
{
    QWidget* theParent = parentWidget();
    Q_ASSERT(theParent != NULL);
                    
    // Set the current animation state
    layoutGrid(theParent);
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getLayoutInfo()
// 
/// Calculate the layout info. This will require three iterations
/// over the children in the layout.
///     1. Calculate the dimensions of the grid
///     2. Determine which cells contain widgets
///     3. Distribute the sizes and weights among the columns and rows.
//-----------------------------------------------------------------------------
DynamicGridLayoutInfo* 
DynamicGridLayout::getLayoutInfo()
{
    DynamicGridLayoutInfo* layoutInfo = new DynamicGridLayoutInfo();
    QWidget* theWidget = NULL;
    DynamicGridConstraints* constraints = NULL;
    QSize theSize;

    int i = 0;
    int k = 0;
    int px = 0; 
    int py = 0; 
    int pixels_diff = 0;
    int nextSize = 0;
    int curX = 0;
    int curY = 0;
    int curWidth = 0;
    int curHeight = 0;
    int curCol = 0;
    int curRow = 0; 
    float weight = 0.0;
    float weightDiff = 0.0;

    // First: Calculate the dimensions of the layout grid.
    curRow = curCol = -1;
    
    int* xMax = new int[kMaxGridSize];
    int* yMax = new int[kMaxGridSize];
    memset(xMax, 0, kMaxGridSize);
    memset(yMax, 0, kMaxGridSize);

    GridConstIterator iter(mItems);    
    while(iter.hasNext()) {
    
        iter.next();       
        theWidget = iter.key();
        constraints = iter.value();

        curX = constraints->mGridX;
        curY = constraints->mGridY;

        curWidth = qMax(1, constraints->width());
        curHeight = qMax(1, constraints->height());
        
        Q_ASSERT(curX >= 0);
        Q_ASSERT(curY >= 0);

#if 0        
        // If x or y is negative, then use relative positioning.
        if (curX < 0 && curY < 0) {
            if (curRow >= 0)
                curY = curRow;
            else if (curCol >= 0)
                curX = curCol;
            else
                curY = 0;
        }
        
        if (curX < 0) {
            px = 0;
            for (i = curY; i < (curY + curHeight); i++)
                px = qMax(px, xMax[i]);

            curX = px - curX - 1;
            if(curX < 0)
                curX = 0;
        } else if (curY < 0) {
            py = 0;
            for (i = curX; i < (curX + curWidth); i++)
                py = qMax(py, yMax[i]);

            curY = py - curY - 1;
            if(curY < 0)
                curY = 0;
        }
#endif

        // Adjust the grid width and height
        for (px = curX + curWidth; layoutInfo->width < px; layoutInfo->width++) {
        }
                
        for (py = curY + curHeight; layoutInfo->height < py; layoutInfo->height++) {
        }
            
        // Adjust the xMax and yMax arrays
        for (i = curX; i < (curX + curWidth); i++) 
            yMax[i] = py; 
        
        for (i = curY; i < (curY + curHeight); i++)
            xMax[i] = px;

        theSize = theWidget->sizeHint();

        constraints->mMinWidth = theSize.width();
        constraints->mMinHeight = theSize.height();

        // If we find a width and height of zero, this is the last item.
        if (constraints->height() == 0 && constraints->width() == 0)
            curRow = curCol = -1;

        // Zero width starts a new row
        if (constraints->height() == 0 && curRow < 0)
            curCol = curX + curWidth;

        // Zero height starts a new column
        else if (constraints->width() == 0 && curCol < 0)
            curRow = curY + curHeight;
    }

    // Apply minimum row/column dimensions
    //if (layoutInfo->width < mColumnWidthsLength)
    //    layoutInfo->width = mColumnWidthsLength;
        
    //if (layoutInfo->height < mRowHeightsLength)
    //    layoutInfo->height = mRowHeightsLength;

    // Free the coordinate arrays
    delete xMax;
    delete yMax;

    // Second:
    // Negative values for mGridX are filled in with the current x value.
    // Negative values for mGridY are filled in with the current y value.
    // Negative or zero values for mGridWidth and mGridHeight end the current
    // row or column, respectively.

    curRow = curCol = -1;
    xMax = new int[kMaxGridSize];
    yMax = new int[kMaxGridSize];
    memset(xMax, 0, kMaxGridSize);
    memset(yMax, 0, kMaxGridSize);

    iter.toFront();
    while(iter.hasNext()) {

        iter.next();       
        theWidget = iter.key();
        constraints = iter.value();

        curX = constraints->mGridX;
        curY = constraints->mGridY;
        curWidth = constraints->width();
        curHeight = constraints->height();

        Q_ASSERT(curX >= 0);
        Q_ASSERT(curY >= 0);

#if 0
        // If x or y is negative, then use a relative position.
        if (curX < 0 && curY < 0) {
            if(curRow >= 0)
                curY = curRow;
            else if(curCol >= 0)
                curX = curCol;
            else
                curY = 0;
        }

        if (curX < 0) {
            if (curHeight <= 0) {
                curHeight += layoutInfo->height - curY;
                if (curHeight < 1)
                    curHeight = 1;
            }

            px = 0;
            for (i = curY; i < (curY + curHeight); i++)
                px = qMax(px, xMax[i]);

            curX = px - curX - 1;
            if(curX < 0)
                curX = 0;
        } else if (curY < 0) {
            if (curWidth <= 0) {
                curWidth += layoutInfo->width - curX;
                if (curWidth < 1)
                    curWidth = 1;
            }

            py = 0;
            for (i = curX; i < (curX + curWidth); i++)
                py = qMax(py, yMax[i]);

            curY = py - curY - 1;
            if(curY < 0)
                curY = 0;
        }
#endif

        if (curWidth <= 0) {
            curWidth += layoutInfo->width - curX;
            if (curWidth < 1)
                curWidth = 1;
        }

        if (curHeight <= 0) {
            curHeight += layoutInfo->height - curY;
            if (curHeight < 1)
                curHeight = 1;
        }

        px = curX + curWidth;
        py = curY + curHeight;

        for (i = curX; i < (curX + curWidth); i++)
            yMax[i] = py;
            
        for (i = curY; i < (curY + curHeight); i++)
            xMax[i] = px;

        // A negative size will begin a new row or column
        if (constraints->height() == 0 && constraints->width() == 0)
            curRow = curCol = -1;
            
        if (constraints->height() == 0 && curRow < 0)
            curCol = curX + curWidth;            
        else if (constraints->width() == 0 && curCol < 0)
            curRow = curY + curHeight;

        // Assign the values to the widget
        constraints->mTempX = curX;
        constraints->mTempY = curY;
        constraints->mTempWidth = curWidth;
        constraints->mTempHeight = curHeight;
    }

    // Apply minimum column and row weights and dimensions
    //memcpy(mColumnWidths, layoutInfo->mMinWidth,  mColumnWidthsLength);
    //memcpy(mRowHeights,layoutInfo->mMinHeight, mRowHeightsLength);
    //memcpy(mColumnWeights, layoutInfo->mWeightX, mColumnWeightsLength);
    //memcpy(mRowWeights,  layoutInfo->mWeightY,  mRowWeightsLength);

    // Third: Distribute the minimun widths and weights:
    nextSize = INT_MAX;

    for (i = 1;
         i != INT_MAX;
         i = nextSize, nextSize = INT_MAX) {

        GridIterator it3(mItems);       
        while (it3.hasNext()) {

            it3.next();       
            theWidget = it3.key();
            constraints = it3.value();

            if (constraints->mTempWidth == i) {
                // right column
                px = constraints->mTempX + constraints->mTempWidth;

                // Figure out if we should use the widget weight/
                // If the weight is less than the total
                // weight spanned by the width of the cell, then
                // discard the weight.  Otherwise split the
                // difference according to the existing weights.
                weightDiff  = constraints->mWeightX;

                for (k = constraints->mTempX; k < px; k++)
                    weightDiff  -= layoutInfo->mWeightX[k];

                if (weightDiff  > 0.0) {
                    weight = 0.0;
                    for (k = constraints->mTempX; k < px; k++)
                        weight += layoutInfo->mWeightX[k];

                    for (k = constraints->mTempX; weight > 0.0 && k < px; k++) {
                        float wt = layoutInfo->mWeightX[k];
                        float dx = (wt * weightDiff ) / weight;
                        layoutInfo->mWeightX[k] += dx;
                        weightDiff  -= dx;
                        weight -= wt;
                    }
                    // Assign the remainder to the rightmost cell.
                    layoutInfo->mWeightX[px-1] += weightDiff ;
                }

                // Calculate the mMinWidth array values.
                // First, figure out how wide the current widget needs to be.
                // Then, see if it will fit within the current minimum width values.
                // If it will not fit, add the difference according to the
                // weight x array.
                pixels_diff = constraints->mMinWidth
                            + constraints->mPadX
                            + constraints->insets().left()
                            + constraints->insets().right();

                for (k = constraints->mTempX; k < px; k++)
                    pixels_diff -= layoutInfo->mMinWidth[k];
                    
                if (pixels_diff > 0) {
                    weight = 0.0;
                    for (k = constraints->mTempX; k < px; k++)
                        weight += layoutInfo->mWeightX[k];
                    for (k = constraints->mTempX; weight > 0.0 && k < px; k++) {
                        float wt = layoutInfo->mWeightX[k];
                        int dx = (int)((wt * ((float)pixels_diff)) / weight);
                        layoutInfo->mMinWidth[k] += dx;
                        pixels_diff -= dx;
                        weight -= wt;
                    }
                    
                    // Left over values will go into the rightmost cell./
                    layoutInfo->mMinWidth[px-1] += pixels_diff;
                }
            } else if (constraints->mTempWidth > i && constraints->mTempWidth < nextSize) {
                nextSize = constraints->mTempWidth;
            }

            if (constraints->mTempHeight == i) {
                py = constraints->mTempY + constraints->mTempHeight;
                // bottom row

                // Figure out if we should use the widget weight.
                // If the weight is less than the total weight spanned 
                // by the height of the cell, then discard the weight.  
                // Otherwise split it the difference according to the existing weights.
                weightDiff  = constraints->mWeightY;
                for (k = constraints->mTempY; k < py; k++)
                    weightDiff  -= layoutInfo->mWeightY[k];
                if (weightDiff  > 0.0) {
                    weight = 0.0;
                    for (k = constraints->mTempY; k < py; k++)
                        weight += layoutInfo->mWeightY[k];
                    for (k = constraints->mTempY; weight > 0.0 && k < py; k++) {
                        float wt = layoutInfo->mWeightY[k];
                        float dy = (wt * weightDiff ) / weight;
                        layoutInfo->mWeightY[k] += dy;
                        weightDiff  -= dy;
                        weight -= wt;
                    }
                    
                    // Assign the remainder to the bottom cell
                    layoutInfo->mWeightY[py-1] += weightDiff ;
                }

                // Calculate the mMinHeight array values.
                // First, figure out how tall the current widget needs to be.
                // Then, see if it will fit within the current mMinHeight values.
                // If it will not fit, add the difference according to the
                // mWeightY array.
                pixels_diff = constraints->mMinHeight
                            + constraints->mPadY
                            + constraints->insets().top()
                            + constraints->insets().bottom();
                            
                for (k = constraints->mTempY; k < py; k++)
                    pixels_diff -= layoutInfo->mMinHeight[k];
                    
                if (pixels_diff > 0) {
                    weight = 0.0;
                    
                    for (k = constraints->mTempY; k < py; k++)
                        weight += layoutInfo->mWeightY[k];
                        
                    for (k = constraints->mTempY; weight > 0.0 && k < py; k++) {
                        float wt = layoutInfo->mWeightY[k];
                        int dy = (int)((wt * ((float)pixels_diff)) / weight);
                        layoutInfo->mMinHeight[k] += dy;
                        pixels_diff -= dy;
                        weight -= wt;
                    }
                    
                    // Assign the remainder to the bottom cell
                    layoutInfo->mMinHeight[py-1] += pixels_diff;
                }
            }
            else if (constraints->mTempHeight > i && constraints->mTempHeight < nextSize)
                nextSize = constraints->mTempHeight;
        }

    }

    // Free the coordinate arrays
    delete xMax;
    delete yMax;

    return layoutInfo;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::adjustForGravity()
//
/// Adjust the various fields to the proper values bases on the
/// layout constaints and padding.
//-----------------------------------------------------------------------------
void 
DynamicGridLayout::adjustForGravity(DynamicGridConstraints* constraints, 
                                    QRect& ioRect)
{
    ioRect.setX(ioRect.x() + constraints->insets().left());
    ioRect.setWidth(ioRect.width() - (constraints->insets().left() + constraints->insets().right()));

    ioRect.setY(ioRect.y() + constraints->insets().top());
    ioRect.setHeight(ioRect.height() - (constraints->insets().top() + constraints->insets().bottom()));

    int diffx = 0;
    if ((  constraints->mFill != DynamicGridConstraints::HORIZONTAL
        && constraints->mFill != DynamicGridConstraints::BOTH)
        && (ioRect.width() > (constraints->mMinWidth + constraints->mPadX))) {
        diffx = ioRect.width() - (constraints->mMinWidth + constraints->mPadX);
        ioRect.setWidth(constraints->mMinWidth + constraints->mPadX);
    }

    int diffy = 0;
    if ((  constraints->mFill != DynamicGridConstraints::VERTICAL
        && constraints->mFill != DynamicGridConstraints::BOTH)
        && (ioRect.height() > (constraints->mMinHeight + constraints->mPadY))) {
        diffy = ioRect.height() - (constraints->mMinHeight + constraints->mPadY);
        ioRect.setHeight(constraints->mMinHeight + constraints->mPadY);
    }

    switch (constraints->mAnchor) {
        case DynamicGridConstraints::CENTER:
            ioRect.setX(ioRect.x() + (diffx / 2));
            ioRect.setY(ioRect.y() + (diffy / 2));
            break;
            
        case DynamicGridConstraints::NORTH:
            ioRect.setX(ioRect.x() + (diffx / 2));
            break;
            
        case DynamicGridConstraints::NORTHEAST:
            ioRect.setX(ioRect.x() + diffx);
            break;
            
        case DynamicGridConstraints::EAST:
            ioRect.setX(ioRect.x() + diffx);
            ioRect.setY(ioRect.y() + (diffy / 2));
            break;
            
        case DynamicGridConstraints::SOUTHEAST:
            ioRect.setX(ioRect.x() + diffx);
            ioRect.setY(ioRect.y() + diffy);
            break;
            
        case DynamicGridConstraints::SOUTH:
            ioRect.setX(ioRect.x() + (diffx / 2));
            ioRect.setY(ioRect.y() + diffy);
            break;
            
        case DynamicGridConstraints::SOUTHWEST:
            ioRect.setY(ioRect.y() + diffy);
            break;
            
        case DynamicGridConstraints::WEST:
            ioRect.setY(ioRect.y() + (diffy / 2));
            break;
            
        case DynamicGridConstraints::NORTHWEST:
            break;
            
        default:
            break;
    }
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getMinSize()
//
/// Calcluate the minimum size based on the results of getLayoutInfo()
/// \sa getLayoutInfo
//-----------------------------------------------------------------------------
QSize
DynamicGridLayout::getMinSize(QWidget* inWidget, DynamicGridLayoutInfo* inInfo)
{
    Q_UNUSED(inWidget);

    QSize theSize;
    
    int i;
    int t = 0;
    for(i = 0; i < inInfo->width; i++)
        t += inInfo->mMinWidth[i];
    theSize.setWidth(t + mContainerInsets.left() + mContainerInsets.right());

    t = 0;
    for(i = 0; i < inInfo->height; i++)
        t += inInfo->mMinHeight[i];
    theSize.setHeight(t + mContainerInsets.top() + mContainerInsets.bottom());

    return theSize;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::layoutGrid()
//
/// Do the layout.
//-----------------------------------------------------------------------------
void 
DynamicGridLayout::layoutGrid(QWidget* inParent)
{
    // Check and see if the parent has children. If it does not,
    // don't set the parent size.
    if (inParent->children().isEmpty())
        return;

    // Iterate all children to figure out the total amount of space needed.
    DynamicGridLayoutInfo* info = getLayoutInfo();
    QSize minSize = getMinSize(inParent, info);

    if (minSize.width() < inParent->width() || minSize.height() < inParent->height()) {
        delete info;
        info = getLayoutInfo();
        minSize = getMinSize(inParent, info);
    }
    
    // Free the old info and set the new info
    delete mLayoutInfo;
    mLayoutInfo = info;

    QRect theBounds;
    theBounds.setWidth(minSize.width());
    theBounds.setHeight(minSize.height());

    Insets insets = mContainerInsets;
    int index = 0;
    float weight = 0;

    // If the current dimensions of the window don't match the desired
    // dimensions, then adjust the mMinWidth and mMinHeight arrays
    // according to the weights.
    int diffw = inParent->width() - theBounds.width();
    if (diffw != 0) {
        weight = 0.0;
        for (index = 0; index < mLayoutInfo->width; index++)
            weight += mLayoutInfo->mWeightX[index];
            
        if (weight > 0.0) {
            for (index = 0; index < mLayoutInfo->width; index++) {
                int dx = (int)(( ((float)diffw) * mLayoutInfo->mWeightX[index]) / weight);
                mLayoutInfo->mMinWidth[index] += dx;
                theBounds.setWidth(theBounds.width() + dx);
                
                if (mLayoutInfo->mMinWidth[index] < 0) {
                    theBounds.setWidth(theBounds.width() - mLayoutInfo->mMinWidth[index]);
                    mLayoutInfo->mMinWidth[index] = 0;
                }
            }
        }
        diffw = inParent->width() - theBounds.width();
    }
    else {
        diffw = 0;
    }

    int diffh = inParent->height() - theBounds.height();
    if (diffh != 0) {
        weight = 0.0;
        for (index = 0; index < mLayoutInfo->height; index++)
            weight += mLayoutInfo->mWeightY[index];
            
        if (weight > 0.0) {
            for (index = 0; index < mLayoutInfo->height; index++) {
                int dy = (int)(( ((float)diffh) * mLayoutInfo->mWeightY[index]) / weight);
                mLayoutInfo->mMinHeight[index] += dy;
                theBounds.setHeight(theBounds.height() + dy);
                
                if (mLayoutInfo->mMinHeight[index] < 0) {
                    theBounds.setHeight(theBounds.height() - mLayoutInfo->mMinHeight[index]);
                    mLayoutInfo->mMinHeight[index] = 0;
                }
            }
        }
        diffh = inParent->height() - theBounds.height();
    }
    else {
        diffh = 0;
    }

    // Do the actual layout of the children using the layout information 
    // that has been calculated.
    mLayoutInfo->startx = diffw / 2 + insets.left();
    mLayoutInfo->starty = diffh / 2 + insets.top();
    
    // Iterate over the managed children
    GridIterator it2(mItems);

    while (it2.hasNext()) {
        it2.next();                   
        QWidget* theWidget = it2.key();
        DynamicGridConstraints* constraints = it2.value();

        theBounds.setX(mLayoutInfo->startx);
        for (index = 0; index < constraints->mTempX; index++)
           theBounds.setX(theBounds.x() + mLayoutInfo->mMinWidth[index]);


        theBounds.setY(mLayoutInfo->starty);
        for(index = 0; index < constraints->mTempY; index++)
            theBounds.setY(theBounds.y() + mLayoutInfo->mMinHeight[index]);

        theBounds.setWidth(0);
        for(index = constraints->mTempX; index < (constraints->mTempX + constraints->mTempWidth); index++) {
            theBounds.setWidth(theBounds.width() + mLayoutInfo->mMinWidth[index]);
        }


        theBounds.setHeight(0);
        for(index = constraints->mTempY; index < (constraints->mTempY + constraints->mTempHeight); index++) {
            theBounds.setHeight(theBounds.height() + mLayoutInfo->mMinHeight[index]);
        }

        adjustForGravity(constraints, theBounds);

        // If the widget is too small, resize it so it is not visible.
        if ((theBounds.width() <= 0) || (theBounds.height() <= 0)) {
            theWidget->setGeometry(0, 0, 0, 0);
            theWidget->hide();
        }
        else {
            if (theWidget->geometry() != theBounds) {
                // Animate the widget to the new location and size.
                mWidgetAnimator.animate(theWidget, 
                                        theBounds,
                                        mAnimate);
            }
        }
    }
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getGridInfo()
//-----------------------------------------------------------------------------
DynamicGridLayoutInfo* 
DynamicGridLayout::getGridInfo(QWidget* inParent)
{
    // Check and see if the parent has children. If it does not,
    // don't set the parent size.
    if (inParent->children().isEmpty())
        return NULL;

    int index = 0;

    // First: Iterate all the children to calcuate total space needed.
    DynamicGridLayoutInfo* info = getLayoutInfo();
    QSize theSize = getMinSize(inParent, info);

    if (theSize.width() < inParent->width() || theSize.height() < inParent->height()) {
        delete info;
        info = getLayoutInfo();
        theSize = getMinSize(inParent, info);
    }

    mLayoutInfo = info;

    QRect r;
    r.setWidth(theSize.width());
    r.setHeight(theSize.height());

    // If the current dimensions of the window don't match the desired
    // dimensions, then adjust the mMinWidth and mMinHeight arrays
    // according to the weights.
    float weight = 0;
    int diffw = inParent->width() - r.width();
    if (diffw != 0) {
        weight = 0.0;
        for (index = 0; index < info->width; index++)
            weight += info->mWeightX[index];
        if (weight > 0.0) {
            for (index = 0; index < info->width; index++) {
                int dx = (int)(( ((float)diffw) * info->mWeightX[index]) / weight);
                info->mMinWidth[index] += dx;
                r.setWidth(r.width() + dx);
                if (info->mMinWidth[index] < 0) {
                    r.setWidth(r.width() - info->mMinWidth[index]);
                    info->mMinWidth[index] = 0;
                }
            }
        }
        diffw = inParent->width() - r.width();
    }
    else {
        diffw = 0;
    }

    int diffh = inParent->height() - r.height();
    if (diffh != 0) {
        weight = 0.0;
        
        for (index = 0; index < info->height; index++)
            weight += info->mWeightY[index];
            
        if (weight > 0.0) {
            for (index = 0; index < info->height; index++) {
                int dy = (int)(( ((float)diffh) * info->mWeightY[index]) / weight);
                info->mMinHeight[index] += dy;
                r.setHeight(r.height() + dy);
                if (info->mMinHeight[index] < 0) {
                    r.setHeight(r.height() - info->mMinHeight[index]);
                    info->mMinHeight[index] = 0;
                }
            }
        }
        diffh = inParent->height() - r.height();
    }
    else {
        diffh = 0;
    }

    return info;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::columnCount()
//
/// Return the number of columns in the layout.
/// \result The column count.
//-----------------------------------------------------------------------------
int
DynamicGridLayout::columnCount()
{
    // Make sure the layout is up to date.
    updateLayout();
    
    // Return the column count    
    return mLayoutInfo->width;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::rowCount()
//
/// Return the number of rows in the layout.
/// \result The row count.
//-----------------------------------------------------------------------------
int
DynamicGridLayout::rowCount()
{
    // Make sure the layout is up to date.
    updateLayout();
    
    // Return the column count    
    return mLayoutInfo->height;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::getConstraints()
//
/// Get the information for the item at the specified location
/// \param inX The x coordinate
/// \param inY The y coordinate
/// \result The layout constraints, or NULL if none are found.
//-----------------------------------------------------------------------------
DynamicGridConstraints*
DynamicGridLayout::getConstraints(int inX, int inY)
{
    if (inX >= columnCount())
        return NULL;

    if (inY >= rowCount())
        return NULL;

    // There should only be a single item at the location
    // or spanning into the location. The layout does not
    // support overlapping items.
    QPoint location(inX, inY);
    
    Q_FOREACH(DynamicGridConstraints* theConstraint, mItems) {
        if (theConstraint->bounds().contains(location))
            return theConstraint;
    }

    return NULL;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::compressLayout()
//
/// Compress the layout to remove the empty item.
/// Call updateLayout() if you want the changed to be visible.
/// \param inBounds The bounds of the item being removed.
/// \sa updateLayout()
//-----------------------------------------------------------------------------
void
DynamicGridLayout::compressLayout(const QRect& inBounds)
{
    if (inBounds.isNull())
        return;
    
    // Get the layout dimensions    
    const int currentColumns = columnCount();
    
    // Get the grid location that is empty
    const int x = inBounds.left();            
    const int y = inBounds.top();            
    
    // Move items to fill the empty columns
    for (int colIndex = x; colIndex < currentColumns; ++colIndex) {
        DynamicGridConstraints* constraint = getConstraints(colIndex + 1, y);
        if (constraint != NULL)
            constraint->setX(constraint->x() - 1);
    }
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::fillEmptySpace()
// 
/// Fill all empty spaces in the grid by resizing items to fill the space.
/// \param inPanelGroup The panel group to remove and delete.
//-----------------------------------------------------------------------------
void
DynamicGridLayout::fillEmptySpace(Qt::Orientation inOrientation)
{
    Q_UNUSED(inOrientation);

    // Now that the item has been added, we need to handle
    // empty grid spaces. Depending on the type of orientation
    // of the item being added, we need to collapse the empty spaces.
    const int numColumns = columnCount();
    const int numRows = rowCount();

    // Get a list of all of the empty grid locations
    QVector<QPoint> emptyList;
    for (int colIndex = 0; colIndex < numColumns; ++colIndex) {
        for (int rowIndex = 0; rowIndex < numRows; ++rowIndex) {
            DynamicGridConstraints* theItem = getConstraints(colIndex, rowIndex);
            if (theItem == NULL)
                emptyList.push_back(QPoint(colIndex, rowIndex));
        }
    }
    
    Q_FOREACH(const QPoint& emptySpace, emptyList) {
        if (inOrientation == Qt::Horizontal) {
            if (numColumns > 0) {
                if (emptySpace.x() == 0) {
                    DynamicGridConstraints* nextItem = getConstraints(emptySpace.x() + 1, emptySpace.y());
                    if (nextItem != NULL) {
                        nextItem->setX(nextItem->x() - 1);
                        nextItem->setWidth(nextItem->width() + 1);
                    }
                } else {
                    DynamicGridConstraints* prevItem = getConstraints(emptySpace.x() - 1, emptySpace.y());
                    if (prevItem != NULL)
                        prevItem->setWidth(prevItem->width() + 1);
                }
            }    
        } else {
            if (numRows > 0) {
                if (emptySpace.y() == 0) {
                    // Pull the items in the row below up to fill the gaps
                    GridConstIterator iter(getConstraintsMap());
                    while (iter.hasNext()) {
                        iter.next();       
                        DynamicGridConstraints* columnItem = iter.value();                        
                        Q_ASSERT(columnItem != NULL);
                        if (columnItem->y() == 1) {
                            
                        }
                    }                    
#if 0                    
                    DynamicGridConstraints* nextItem = getConstraints(emptySpace.x(), emptySpace.y() + 1);
                    if (nextItem != NULL) {
                        nextItem->setY(nextItem->y() - 1);
                        nextItem->setHeight(nextItem->height() + 1);
                    }
#endif
                } else {
                    DynamicGridConstraints* prevItem = getConstraints(emptySpace.x(), emptySpace.y() - 1);
                    if (prevItem != NULL)
                        prevItem->setHeight(prevItem->height() + 1);
                }
            }    
        }
    }
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::beginDeferLayout()
//
/// Defer layout until endDeferLayout() is called.
/// Note: updateLayout() does not check this flag, rather other code
/// paths do.  We do not check the flag in updateLayout() as internal code
/// needs to call updateLayout() internally.
/// \sa endDeferLayout
//-----------------------------------------------------------------------------
void
DynamicGridLayout::beginDeferLayout()
{
    // Defer layou
    mDeferLayout = true;
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::endDeferLayout()
//
/// Allow layout. updateLayout() will be called by this method.
/// \sa updateLayout
//-----------------------------------------------------------------------------
void
DynamicGridLayout::endDeferLayout()
{
    mDeferLayout = false;
    
    updateLayout();
    
    fillEmptySpace();
}


//-----------------------------------------------------------------------------
// DynamicGridLayout::dumpLayout()
//
/// Debugging routine to display occupied grid positions.
//-----------------------------------------------------------------------------
void
DynamicGridLayout::dumpLayout(const QString& inMessage)
{
    const int numCols = columnCount();
    const int numRows = rowCount();

    if (!inMessage.isEmpty())
        printf("%s\n", inMessage.toStdString().c_str());
    
    printf("columns: %d     rows: %d\n", numCols, numRows);

    // Print the layout
    for (int rowIndex = 0; rowIndex < numRows; ++rowIndex) {
        for (int colIndex = 0; colIndex < numCols; ++colIndex) {
            DynamicGridConstraints* constraint = getConstraints(colIndex, rowIndex);
            if (constraint != NULL) {
                printf("x ");
                //qDebug() << constraint->bounds();
            } else {
                printf("- ");
            }                    
        }
        printf("\n");
    }
    printf("\n\n");
}

