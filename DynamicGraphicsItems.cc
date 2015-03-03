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
#include "DynamicGraphicsItems.h"

// Qt
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneDragDropEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>

// Constants
static const int kPrelight = 180;
static const int kHover = 780;


//=============================================================================
// class DynamicPixmapItem
//=============================================================================

//-----------------------------------------------------------------------------
// DynamicPixmapItem::DynamicPixmapItem
//-----------------------------------------------------------------------------
DynamicPixmapItem::DynamicPixmapItem(const QPixmap& inPixmap,
                                     QGraphicsItem* inParent)
    :    QGraphicsPixmapItem(inPixmap, inParent),
        mAnimation(NULL),
        mEffect(NULL)
{
    // Allow mouse over actions
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    // Set up the animation
    mAnimation = new QPropertyAnimation(this, "pos");
    mAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Set up the prelighting effect
    mEffect = new QGraphicsColorizeEffect();
    setGraphicsEffect(mEffect);
    mEffect->setColor(Qt::white);
    mEffect->setEnabled(false);
}


//-----------------------------------------------------------------------------
// DynamicPixmapItem::hoverEnterEvent
//-----------------------------------------------------------------------------
void
DynamicPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent* inEvent)
{
    Q_UNUSED(inEvent);
    mEffect->setEnabled(true);
}


//-----------------------------------------------------------------------------
// DynamicPixmapItem::hoverLeaveEvent
//-----------------------------------------------------------------------------
void
DynamicPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* inEvent)
{
    Q_UNUSED(inEvent);
    mEffect->setEnabled(false);
}


//-----------------------------------------------------------------------------
// DynamicPixmapItem::mousePressEvent
//-----------------------------------------------------------------------------
void
DynamicPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* inEvent)
{
    Q_UNUSED(inEvent);
}


//-----------------------------------------------------------------------------
// DynamicPixmapItem::mouseReleaseEvent
//-----------------------------------------------------------------------------
void
DynamicPixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* inEvent)
{
     QGraphicsItem::mouseReleaseEvent(inEvent);

    Q_EMIT clicked(this);
}


//=============================================================================
// class DynamicPolygonItem
//=============================================================================

//-----------------------------------------------------------------------------
// DynamicPolygonItem::DynamicPolygonItem
//-----------------------------------------------------------------------------
DynamicPolygonItem::DynamicPolygonItem(QGraphicsItem* inParent)
    :    QGraphicsPolygonItem(inParent),
        mPosAnimation(NULL),
        mPrelight(false),
        mThrob(false)
{
    // Allow mouse over actions
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    // Set up the position animation
    mPosAnimation = new QPropertyAnimation(this, "pos");
    mPosAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // The default colors
    mBackgroundColor = QColor(255, 255, 255, 100);
    mPrelightColor = QColor(142, 172, 255, 100);
    mHoverColor = QColor(199, 214, 255, 100);
    mFadeColor = QColor(255, 255, 255, 100);

    // Set the default colors
    setPen(QColor(128, 128, 128, 100));
    setBrush(mBackgroundColor);
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::setFadeColor
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::setFadeColor(const QColor& inColor)
{
    mFadeColor = inColor;
    setBrush(mFadeColor);
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::dragEnterEvent
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::dragEnterEvent(QGraphicsSceneDragDropEvent* inEvent)
{
    Q_UNUSED(inEvent);
    qDebug("dragEnterEvent");
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::dragLeaveEvent
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::dragLeaveEvent(QGraphicsSceneDragDropEvent* inEvent)
{
    Q_UNUSED(inEvent);
    qDebug("dragLeaveEvent");
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::hoverEnterEvent
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::hoverEnterEvent(QGraphicsSceneHoverEvent* inEvent)
{
    Q_UNUSED(inEvent);
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::hoverLeaveEvent
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* inEvent)
{
    Q_UNUSED(inEvent);
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::mousePressEvent
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::mousePressEvent(QGraphicsSceneMouseEvent* inEvent)
{
    Q_UNUSED(inEvent);
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::mouseReleaseEvent
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* inEvent)
{
     QGraphicsItem::mouseReleaseEvent(inEvent);

    Q_EMIT clicked(this);
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::setPrelight
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::setPrelight(bool inPrelight)
{
    if (inPrelight == mPrelight)
        return;

    mPrelight = inPrelight;

    // Clear out the old animations
    mAnimationGroup.stop();
    mAnimationGroup.clear();

    // Set up the prelight animation
    QPropertyAnimation* prelightAnimation = new QPropertyAnimation(this, "fadeColor");
    prelightAnimation->setDuration(kPrelight);
    mAnimationGroup.addAnimation(prelightAnimation);

    if (mPrelight) {
        // Fade into the prelight tint
        prelightAnimation->setStartValue(mBackgroundColor);
        prelightAnimation->setEndValue(mPrelightColor);

        // Set up the hover throbber
        if (mThrob) {
            QPropertyAnimation* hoverAnimation = new QPropertyAnimation(this, "fadeColor");
            hoverAnimation->setEasingCurve(QEasingCurve::InOutQuad);
            hoverAnimation->setDuration(kHover);
            hoverAnimation->setStartValue(mPrelightColor);
            hoverAnimation->setKeyValueAt(0.5, mHoverColor);
            hoverAnimation->setEndValue(mPrelightColor);
            hoverAnimation->setLoopCount(-1);
            mAnimationGroup.addAnimation(hoverAnimation);
        }
    } else {
        // Fade into the normal color
        setBrush(mBackgroundColor);
        prelightAnimation->setStartValue(mPrelightColor);
        prelightAnimation->setEndValue(mBackgroundColor);
    }

    mAnimationGroup.start();
}


//-----------------------------------------------------------------------------
// DynamicPolygonItem::stopPrelight
//-----------------------------------------------------------------------------
void
DynamicPolygonItem::stopPrelight()
{
    mPrelight = false;

    // Stop the animations
    mAnimationGroup.stop();
    mAnimationGroup.clear();

    setBrush(mBackgroundColor);
}


//=============================================================================
// class DynamicRectItem
//=============================================================================

//-----------------------------------------------------------------------------
// DynamicRectItem::DynamicRectItem
//-----------------------------------------------------------------------------
DynamicRectItem::DynamicRectItem(qreal inX,
                                   qreal inY,
                                   qreal inWidth,
                                   qreal inHeight,
                                   QGraphicsItem* inParent)
    :    QGraphicsRectItem(inX, inY, inWidth, inHeight, inParent)
{
    setAcceptDrops(true);
}

