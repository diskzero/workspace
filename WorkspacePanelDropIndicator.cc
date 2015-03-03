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
#include "WorkspacePanelDropIndicator.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QPaintEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPen>
#include <QtDebug>

// Local
#include "DynamicGraphicsItems.h"


// Debugging defines. Uncomment to enable verbose
//#define DEBUG_DROP_AREA

// Constants
namespace {
static const int kFadeInSpeed = 300;
static const int kFadeOutSpeed = 200;
static const qreal kStartFadeValue = 0.50;
static const qreal kEndFadeValue = 0.25;
static const qreal kFadeOutValue = 0.10;
}

QDebug operator<<(QDebug dbg, WorkspacePanelDropIndicator::Area area)
{
    switch (area){
    case WorkspacePanelDropIndicator::AreaNone   : dbg.nospace() << "none"; break;
    case WorkspacePanelDropIndicator::AreaNorth  : dbg.nospace() << "north"; break;
    case WorkspacePanelDropIndicator::AreaSouth  : dbg.nospace() << "south"; break;
    case WorkspacePanelDropIndicator::AreaEast   : dbg.nospace() << "east"; break;
    case WorkspacePanelDropIndicator::AreaWest   : dbg.nospace() << "west"; break;
    case WorkspacePanelDropIndicator::AreaCentral: dbg.nospace() << "central"; break;
    }
    return dbg;
}

//=============================================================================
// class WorkspacePanelDropIndicator
//=============================================================================
WorkspacePanelDropIndicator::WorkspacePanelDropIndicator(QWidget* parent)
    :   QGraphicsView(parent),
        mPanel(NULL),
        mDropArea(AreaNone),
        mCentralAreaEnabled(true),
        mBackground(NULL),
        mFader(NULL),
        mNorth(NULL),
        mEast(NULL),
        mSouth(NULL),
        mWest(NULL),
        mCentral(NULL),
        mPrelightItem(NULL),
        mAnimating(false),
        mFadeAnimation(NULL)
{
    setObjectName("WorkspacePanelDropIndicator");

    // Don't draw any type of frame
    setFrameStyle(QFrame::NoFrame);
    setFrameShadow(QFrame::Plain);

    // Don't allow scrollbars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set up drawing state
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // Set up the scene
    setScene(&mScene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Add the background image pixmap
    mBackground = mScene.addPixmap(QPixmap());

    // Add the opacity fader item
    mFader = new DynamicRectItem(0, 0, 1, 1);
    mFader->setBrush(QBrush(Qt::black));
    mFader->setPen(Qt::NoPen);
    mFader->setOpacity(kStartFadeValue);
    mScene.addItem(mFader);

    // Add the drop zone polygons
    mNorth = new DynamicPolygonItem();
    mScene.addItem(mNorth);

    mEast = new DynamicPolygonItem();
    mScene.addItem(mEast);

    mSouth = new DynamicPolygonItem();
    mScene.addItem(mSouth);

    mWest = new DynamicPolygonItem();
    mScene.addItem(mWest);

    mCentral = new DynamicPolygonItem();
    mScene.addItem(mCentral);

    // Set up the opacity animation
    mFadeAnimation = new QPropertyAnimation(this);
    mFadeAnimation->setTargetObject(mFader);
    mFadeAnimation->setPropertyName("opacity");
    mFadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Watch for completion of animation
    connect(mFadeAnimation, SIGNAL(finished()), this, SLOT(animationFinished()));

    // Don't show until we are invoked
    hide();
}


WorkspacePanelDropIndicator::~WorkspacePanelDropIndicator()
{
}


void
WorkspacePanelDropIndicator::setPanel(QWidget* inPanel)
{
    if (mPanel == inPanel)
        return;

    mPanel = inPanel;

    if (mPanel != NULL) {
        // reset items states as we move to a new panel
        if (mPrelightItem != NULL) {
            mPrelightItem->stopPrelight();
            mPrelightItem = NULL;
        }
        mFadeAnimation->stop();
        mFader->setOpacity(kStartFadeValue);

        // hide while we update, to prevent flicker
        hide();

        // update geometries
        updateGeometry();
        updateOffscreen();

        // show ourselves again
        show();

        // animate the fade in
        fadeIn();
    } else {
        // final fade out
        fadeOut();
    }
}


void
WorkspacePanelDropIndicator::setDropPos(const QPoint& globalPos)
{
    if (mDropPos == globalPos)
        return;

    mDropPos = globalPos;
    Area area = calcDropArea();
    if (area != mDropArea)
        mDropArea = area;

#ifdef DEBUG_DROP_AREA
    switch (mDropArea) {
        case AreaNone:
            qDebug() << "AreaNone";                                        
            break;

        case AreaNorth:
            qDebug() << "AreaNorth";                                        
            break;

        case AreaSouth:
            qDebug() << "AreaSouth";                                        
            break;

        case AreaEast:
            qDebug() << "AreaEast";                                        
            break;

        case AreaWest:
            qDebug() << "AreaWest";                                        
            break;

        case AreaCentral:
            qDebug() << "AreaCentral";                                        
            break;

        default:
            qDebug() << "Unknown area";                                        
            break;           

    }
#endif            

    prelightDropArea(globalPos);
}


WorkspacePanelDropIndicator::Area
WorkspacePanelDropIndicator::calcDropArea() const
{
    const QRect boundingRect(QPoint(0,0), size());
    const QPoint panelPos = mapFromGlobal(mDropPos);

    // First, figure out if the dropPos is over the central rectangle
    // (for adding to a panel group).
    //const int marginX = boundingRect.width() / 4;
    //const int marginY = boundingRect.height() / 4;
    //if (marginX > 0 && marginY > 0) {
    const QRect centralRect = calculateCentralRect(boundingRect);
    if (centralRect.contains(panelPos)){
        return mCentralAreaEnabled ? AreaCentral : AreaNone;
    }

    // It's not over the central frame. Figure out which quadrant
    // it's in. Calculate the slope of the two dividing lines, and
    // see if position lies above or below each line.
    //
    // NW                    NE
    //  +--------------------+
    //  |\                  /|
    //  | \                / |
    //  |  \              /  |
    //  |   \            /   |
    //  | above     above NW |
    //  | SW to NE  to / SE  |
    //  |      \      /      |
    //  |       \    /       |
    //  |        \  /        |
    //  |         \/         |
    //  |         /\         |
    //  |        /  \        |
    //  |       /    \       |
    //  |      /      \      |
    //  |     /        \     |
    //  |    /          \    |
    //  |   /            \   |
    //  |  /              \  |
    //  | /                \ |
    //  |/                  \|
    //  +--------------------+
    // SW                    SE

    const float slopeToSE = boundingRect.height() / float(boundingRect.width());
    const float slopeToNE = -slopeToSE;

    const bool pointAboveNWtoSE = panelPos.y() <
        (slopeToSE * panelPos.x() + 0         );

    const bool pointAboveSWtoNE = panelPos.y() <
        (slopeToNE * panelPos.x() + boundingRect.height());

    Area area = mDropArea;
    if (pointAboveSWtoNE){
        if (pointAboveNWtoSE){
            area = AreaNorth;
        } else {
            area = AreaWest;
        }
    } else {
        if (pointAboveNWtoSE){
            area = AreaEast;
        } else {
            area = AreaSouth;
        }
    }
    return area;
}


void
WorkspacePanelDropIndicator::prelightDropArea(const QPoint& inPosition)
{
    // Find the item to prelight
    QGraphicsItem* item = itemAt(mapFromGlobal(inPosition));
    if (item == NULL)
        return;

    // Make sure it is one of our zones
    DynamicPolygonItem* polygonItem = dynamic_cast<DynamicPolygonItem*>(item);
    if (polygonItem == NULL)
        return;

    // Check and see if there is anything to do
    if (polygonItem == mPrelightItem)
        return;

    // Check and see if we are over the central area and cannot accept a drop there.
    bool canAcceptDrop = true;
    if (polygonItem == mCentral && mCentralAreaEnabled == false) {
        canAcceptDrop = false;
    }
    
    // Stop the previous hover timer
    mHoverTimer.stop();
    
    // Check and see if we have to clear out the last prelight
    if (polygonItem != NULL) {

        // Turn off previous prelighting
        if (mPrelightItem != NULL) {
            mPrelightItem->setPrelight(false);
        }

        if (canAcceptDrop) {
            // Set the new prelight widget
            mPrelightItem = polygonItem;

            // Draw the new prelighting
            mPrelightItem->setPrelight(true);
        } else {
            // There is no prelight area.
            mPrelightItem = NULL;
        }
    }
}


void
WorkspacePanelDropIndicator::updateGeometry()
{
    setGeometry(mPanel->frameGeometry());
}


//-----------------------------------------------------------------------------
// WorkspacePanelDropIndicator::updateOffscreen
//-----------------------------------------------------------------------------
void
WorkspacePanelDropIndicator::updateOffscreen()
{
    // Adjust our bounds to our parents
    resize(mPanel->width(), mPanel->height());

    // Grab the offscreen of our parent
#ifdef Q_OS_MACX    
    QPixmap pixmap = mPanel->grab(QRect(0,
                                        0,
                                        mPanel->width(),
                                        mPanel->height()));
#else
    QPixmap pixmap = QPixmap::grabWindow(mPanel->winId(),
                                         0,
                                         0,
                                         mPanel->width(),
                                         mPanel->height());
#endif

    mBackground->setPixmap(pixmap);

    // Update the bounds of the opacity fader
    mFader->setRect(0,
                    0,
                    mPanel->width(),
                    mPanel->height());

    // Udpate the drop zone polygons
    QVector<QPointF> pts(4);

    // Calculate the bounding rect
    const QRect boundingRect(QPoint(0,0), mPanel->size());

    // Calculate the central rect
    const QRect centralRect = calculateCentralRect(boundingRect);

    // North
    pts[0] = boundingRect.topLeft();
    pts[1] = boundingRect.topRight();
    pts[2] = centralRect.topRight();
    pts[3] = centralRect.topLeft();
    QPolygonF northPolygon(pts);
    mNorth->setPolygon(northPolygon);

    // East
    pts[0] = boundingRect.topRight();
    pts[1] = boundingRect.bottomRight();
    pts[2] = centralRect.bottomRight();
    pts[3] = centralRect.topRight();
    QPolygonF eastPolygon(pts);
    mEast->setPolygon(eastPolygon);

    // South
    pts[0] = boundingRect.bottomRight();
    pts[1] = boundingRect.bottomLeft();
    pts[2] = centralRect.bottomLeft();
    pts[3] = centralRect.bottomRight();
    QPolygonF southPolygon(pts);
    mSouth->setPolygon(southPolygon);

    // West
    pts[0] = boundingRect.bottomLeft();
    pts[1] = boundingRect.topLeft();
    pts[2] = centralRect.topLeft();
    pts[3] = centralRect.bottomLeft();
    QPolygonF westPolygon(pts);
    mWest->setPolygon(westPolygon);

    // Central
    pts[0] = centralRect.topLeft();
    pts[1] = centralRect.topRight();
    pts[2] = centralRect.bottomRight();
    pts[3] = centralRect.bottomLeft();
    QPolygonF centralPolygon(pts);
    mCentral->setPolygon(centralPolygon);

    // Make sure we are top most and visible
    raise();
    show();
}


//-----------------------------------------------------------------------------
// WorkspacePanelDropIndicator::fadeIn
//-----------------------------------------------------------------------------
void
WorkspacePanelDropIndicator::fadeIn()
{
    mAnimating = true;

    // Set up the fade in amount
    mFadeAnimation->setDuration(kFadeInSpeed);
    mFadeAnimation->setStartValue(kStartFadeValue);
    mFadeAnimation->setEndValue(kEndFadeValue);

    // Show the items
    mFadeAnimation->start();
}


//-----------------------------------------------------------------------------
// WorkspacePanelDropIndicator::fadeOut
//-----------------------------------------------------------------------------
void
WorkspacePanelDropIndicator::fadeOut()
{
    mAnimating = false;

    if (mFadeAnimation->state() == QAbstractAnimation::Running)
        return;

    // Set up the fade out amount
    mFadeAnimation->setDuration(kFadeOutSpeed); 
    mFadeAnimation->setStartValue(kEndFadeValue);
    mFadeAnimation->setEndValue(kEndFadeValue + kFadeOutValue); 

    // Hide the items
    mFadeAnimation->start();
}


//-----------------------------------------------------------------------------
// WorkspacePanelDropIndicator::calculateCentralRect
//
/// Given a bounding rectangle, calculate the central rectangle, 
/// adjusting the margins so the central rectangle appears uniform 
/// when centered in the bounding rectangle. 
/// \param inRect The bounding rectangle.
/// \result The central rectangle.
//-----------------------------------------------------------------------------
QRect 
WorkspacePanelDropIndicator::calculateCentralRect(const QRect& inRect) const
{
   int marginX = std::max(1, inRect.width() / 4);
    int marginY = std::max(1, inRect.height() / 4);

    // We want the margins to appear uniform.
    if (marginX > marginY)
        marginX = marginY;

    if (marginY > marginX)
        marginY = marginX;

    return inRect.adjusted(marginX, marginY, -marginX, -marginY);
}


//-----------------------------------------------------------------------------
// WorkspacePanelDropIndicator::animationFinished    [slot]
//-----------------------------------------------------------------------------
void
WorkspacePanelDropIndicator::animationFinished()
{
    if (!mAnimating) 
        hide();
}
