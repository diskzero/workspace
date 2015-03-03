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

#ifndef WORKSPACE_PANEL_DROP_INDICATOR_HAS_BEEN_INCLUDED
#define WORKSPACE_PANEL_DROP_INDICATOR_HAS_BEEN_INCLUDED

// Qt
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

// Forward declarations
class DynamicPolygonItem;
class DynamicRectItem;
class QGraphicsPixmapItem;
class QPropertyAnimation;

//=============================================================================
// class WorkspacePanelDropIndicator
//=============================================================================

/// \brief A transparent widget that covers a panel to indicate the effects of
/// dropping something on it
///
class WorkspacePanelDropIndicator : public QGraphicsView
{
    Q_OBJECT

public:
    enum Area {
        AreaNone,
        AreaNorth,
        AreaSouth,
        AreaEast,
        AreaWest,
        AreaCentral
    };

    WorkspacePanelDropIndicator(QWidget* parent = 0);
    ~WorkspacePanelDropIndicator();

    // Set the panel that this indicator will cover
    void setPanel(QWidget* panel);

    void enableCentralArea();
    void disableCentralArea();

    // Set the position where the drop would occur
    void setDropPos(const QPoint& globalPos);
    const QPoint& getDropPos() const;

    // Get the Area corresponding to the drop position
    Area getDropArea() const;

    // TODO: We want to get rid of this.
    // Prelighting should be done by the standard event handling
    // mechanism if possible.
    void prelightDropArea(const QPoint& inPosition);

private Q_SLOTS:
    void animationFinished();

private:
    // No copying
    Q_DISABLE_COPY(WorkspacePanelDropIndicator)

    void updateGeometry();
    Area calcDropArea() const;

    void updateOffscreen();

    void fadeIn();
    void fadeOut();

    QRect calculateCentralRect(const QRect& inRect) const;
    
    QWidget* mPanel;
    QPoint mDropPos;
    Area mDropArea;
    bool mCentralAreaEnabled;

    QGraphicsScene mScene;
    QGraphicsPixmapItem* mBackground;
    DynamicRectItem* mFader;
    DynamicPolygonItem* mNorth;
    DynamicPolygonItem* mEast;
    DynamicPolygonItem* mSouth;
    DynamicPolygonItem* mWest;
    DynamicPolygonItem* mCentral;
    DynamicPolygonItem* mPrelightItem;
    bool mAnimating;
    QPropertyAnimation* mFadeAnimation;
    QBasicTimer mHoverTimer;
};

inline const QPoint& WorkspacePanelDropIndicator::getDropPos() const { return mDropPos; }
inline WorkspacePanelDropIndicator::Area WorkspacePanelDropIndicator::getDropArea() const { return mDropArea; }
inline void WorkspacePanelDropIndicator::enableCentralArea() { mCentralAreaEnabled = true; }
inline void WorkspacePanelDropIndicator::disableCentralArea() { mCentralAreaEnabled = false; }

#endif // !WORKSPACE_PANEL_DROP_INDICATOR_HAS_BEEN_INCLUDED

