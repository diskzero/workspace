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
#include "WidgetAnimator.h"

// Qt
#include <QPropertyAnimation>
#include <QtDebug>
#include <QWidget>

// Local
#include "DynamicGridLayout.h"


// Set the default animation time
static const int kAnimationDuration = 200;

//=============================================================================
// class WidgetAnimator
//=============================================================================
WidgetAnimator::WidgetAnimator(DynamicGridLayout* inLayout) 
    :   mLayout(inLayout)
{
}

void WidgetAnimator::abort(QWidget *w)
{
    AnimationMap::iterator it = mAnimationMap.find(w);
    if (it == mAnimationMap.end())
        return;
        
    QPropertyAnimation *anim = *it;
    mAnimationMap.erase(it);
    anim->stop();
    //mLayout->animationFinished(w);
}


void 
WidgetAnimator::animationFinished()
{
    QPropertyAnimation *anim = qobject_cast<QPropertyAnimation*>(sender());
    abort(static_cast<QWidget*>(anim->targetObject()));
}


void 
WidgetAnimator::animationObjectDestroyed()
{
    QPropertyAnimation *anim = static_cast<QPropertyAnimation*>(sender());
    Q_ASSERT(anim != NULL);
    QWidget* animKeyWidget = mAnimationMap.key(anim);
    mAnimationMap.remove(animKeyWidget);
}  


void
WidgetAnimator::animate(QWidget* inWidget, const QRect& inDestination, bool inAnimate)
{
    QRect widgetFrame = inWidget->geometry();
    if (widgetFrame.right() < 0 || widgetFrame.bottom() < 0)
        widgetFrame = QRect();

    inAnimate = inAnimate && !widgetFrame.isNull() && !inDestination.isNull();

    // Might make the widget go away by sending it to negative space
    const QRect finalGeometry = inDestination.isValid() || inWidget->isWindow() ? inDestination :
        QRect(QPoint(-500 - inWidget->width(), -500 - inWidget->height()), inWidget->size());

    // Check and see if we are already at the destination
    if (widgetFrame == finalGeometry)
        return;
        
    AnimationMap::const_iterator it = mAnimationMap.constFind(inWidget);
    if (it != mAnimationMap.constEnd() && NULL != (*it) 
        && (*it)->endValue().toRect() == finalGeometry) {
        return;
    }

    // Set up the property animation    
    QPropertyAnimation* theAnimation = new QPropertyAnimation(inWidget, "geometry", inWidget);
    theAnimation->setDuration(inAnimate ? kAnimationDuration : 0);
    theAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    theAnimation->setStartValue(widgetFrame);
    theAnimation->setEndValue(finalGeometry);
    mAnimationMap[inWidget] = theAnimation;
    connect(theAnimation, SIGNAL(finished()), SLOT(animationFinished()));
    connect(theAnimation, SIGNAL(destroyed()), SLOT(animationObjectDestroyed()));
    theAnimation->start(QPropertyAnimation::DeleteWhenStopped);
}


bool 
WidgetAnimator::animating() const
{
    return !mAnimationMap.isEmpty();
}


bool 
WidgetAnimator::animating(QWidget* inWidget) const
{
    return mAnimationMap.contains(inWidget);
}
