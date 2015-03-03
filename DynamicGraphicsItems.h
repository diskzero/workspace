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

#ifndef DYNAMIC_GRAPHICS_ITEMS_H
#define DYNAMIC_GRAPHICS_ITEMS_H

// Qt
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QSequentialAnimationGroup>

// Forward declarations
class QGraphicsColorizeEffect;
class QPropertyAnimation;


//=============================================================================
// class DynamicPixmapItem
//=============================================================================
class DynamicPixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    DynamicPixmapItem(const QPixmap& inPixmap, QGraphicsItem* inParent = NULL);
    virtual ~DynamicPixmapItem() {}

    QPropertyAnimation* getAnimation() const;

Q_SIGNALS:
    void clicked(DynamicPixmapItem* theItem);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* inEvent);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* inEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* inEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* inEvent);

private:
    QPropertyAnimation* mAnimation;
    QGraphicsColorizeEffect* mEffect;
};

inline QPropertyAnimation* DynamicPixmapItem::getAnimation() const { return mAnimation; }


//=============================================================================
// class DynamicPolygonItem
//=============================================================================
class DynamicPolygonItem : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(QColor fadeColor READ fadeColor WRITE setFadeColor)

public:
    DynamicPolygonItem(QGraphicsItem* inParent = NULL);

    virtual ~DynamicPolygonItem() {}

    void setPrelight(bool inPrelight);
    void stopPrelight();

    QPropertyAnimation* getPosAnimation() const;

    QColor fadeColor() const;
    void setFadeColor(const QColor& inColor);

Q_SIGNALS:
    void clicked(DynamicPolygonItem* theItem);


protected:
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* inEvent);
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* inEvent);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* inEvent);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* inEvent);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* inEvent);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* inEvent);

private:
    QPropertyAnimation* mPosAnimation;
    QSequentialAnimationGroup mAnimationGroup;
    bool mPrelight;
    QColor mBackgroundColor;
    QColor mPrelightColor;
    QColor mHoverColor;
    QColor mFadeColor;
    bool mThrob;
};

inline QColor DynamicPolygonItem::fadeColor() const { return mFadeColor; }
inline QPropertyAnimation* DynamicPolygonItem::getPosAnimation() const { return mPosAnimation; }


//=============================================================================
// class DynamicRectItem
//=============================================================================
class DynamicRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    DynamicRectItem(qreal inX,
                    qreal inY,
                    qreal inWidth,
                    qreal inHeight,
                    QGraphicsItem* inParent = NULL);

};


#endif // DYNAMIC_GRAPHICS_ITEMS_H


