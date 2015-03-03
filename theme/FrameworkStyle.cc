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
#include "FrameworkStyle.h"

// Qt
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QVarLengthArray>

// Namespaces
using namespace workspace;

//=============================================================================
// class FrameworkStyle
//=============================================================================

//-----------------------------------------------------------------------------
// FrameworkStyle::FrameworkStyle()
//-----------------------------------------------------------------------------

FrameworkStyle::FrameworkStyle()
{
    setObjectName("FrameworkStyle");

    // Load in the icons
    mBranchOpenIcon.load(QString::fromUtf8(":/themes/normal/icons/branch-open-default.png"));
    mBranchClosedIcon.load(QString::fromUtf8(":/themes/normal/icons/branch-closed-default.png"));
}


//-----------------------------------------------------------------------------
// FrameworkStyle::~FrameworkStyle()
//-----------------------------------------------------------------------------

FrameworkStyle::~FrameworkStyle()
{
}

//-----------------------------------------------------------------------------
// FrameworkStyle::generatePalette()
//-----------------------------------------------------------------------------

QPalette
FrameworkStyle::generatePalette(int hue, int sat, int val, int contrast) const
{
    double contrastfactor = 1.0 + (contrast * 0.0025);

    val = (int)(val * 0.78) + 48; // avoid extremes

    // compute three basic colors
    QColor window, button, base;
    // window will have just a bit of color
    window.setHsv(hue, sat/16, val);
    // button is darker and more saturated than window
    button.setHsv(hue, sat/8, (int)(val / contrastfactor));
    // base is lighter, less saturated, with no transfats
    int lowsat = qMax((sat-128) / 8, 0);
    base.setHsv(hue, lowsat, qMin((int)(val * contrastfactor), 255));

    // compute text colors
    QColor foreground, brighttext, disabled;

    disabled.setHsv(hue, lowsat, 128);
    if (val < 128) {
        // dark theme
        foreground.setHsv(hue, lowsat, 255 - qMax(75-contrast, 0));
        brighttext.setHsv(hue, lowsat, qMax(75-contrast, 0));
        if (val > 96) disabled.setHsv(hue, lowsat, val + 32);
    } else {
        foreground.setHsv(hue, lowsat, qMax(75-contrast, 0));
        brighttext.setHsv(hue, lowsat, 255 - qMax(75-contrast, 0));
        if (val < 160) disabled.setHsv(hue, lowsat, val - 32);
    }
    
    // compute highlight
    QColor highlight, highlighttext;
    if (val < 128) {
        highlight.setHsv(hue, sat, 255 - qMax(75-contrast, 0));
        highlighttext = brighttext;
    } else {
        // need low value with low saturation
        if (sat < 128) {
            highlight.setHsv(hue, sat, qMax(128-contrast, 0));
            highlighttext = brighttext;
        } else {
            int hisat = qMin((int)(sat * contrastfactor), 255);
            highlight.setHsv(hue, hisat, 255 - qMax(75-contrast, 0));
            highlighttext = foreground;
        }
    }

    // construct palette
    QPalette pal(button);
    pal.setColorGroup(QPalette::Active, foreground, button, button.lighter(150),
                           button.darker(200), button.darker(150), foreground,
                           brighttext, base, window);
    pal.setColorGroup(QPalette::Inactive, foreground, button, button.lighter(150),
                           button.darker(200), button.darker(150), foreground,
                           brighttext, base, window);
    pal.setColorGroup(QPalette::Disabled, disabled, window, window.lighter(150),
                           window.darker(200), window.darker(150), disabled,
                           brighttext, base, window);

    pal.setColor(QPalette::Active, QPalette::Highlight, highlight);
    pal.setColor(QPalette::Inactive, QPalette::Highlight, highlight);
    pal.setColor(QPalette::Disabled, QPalette::Highlight, window);
    pal.setColor(QPalette::Active, QPalette::HighlightedText, highlighttext);
    pal.setColor(QPalette::Inactive, QPalette::HighlightedText, highlighttext);
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, disabled);

    return pal;
}

//-----------------------------------------------------------------------------
// FrameworkStyle::standardPalette()
//-----------------------------------------------------------------------------
QPalette FrameworkStyle::standardPalette() const
{
    return generatePalette(240, 16, 125, 50);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::polish()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::polish(QPalette& palette)
{
    QCommonStyle::polish(palette);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::polish()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::polish(QApplication* application)
{
    QCommonStyle::polish(application);
}    


//-----------------------------------------------------------------------------
// FrameworkStyle::polish()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::polish(QWidget* widget)
{
    QCommonStyle::polish(widget);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::unpolish()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::unpolish(QApplication* application)
{
    QCommonStyle::unpolish(application);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::unpolish()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::unpolish(QWidget* widget)
{
    QCommonStyle::unpolish(widget);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::drawPrimitive()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::drawPrimitive(PrimitiveElement element, 
                           const QStyleOption* option, 
                           QPainter* painter, 
                           const QWidget* widget) const
{
    switch (element) {

        case PE_PanelButtonCommand:
        {
            painter->save();

            const bool enabled = option->state & State_Enabled;
            QRect bounds = option->rect;

            painter->setBrush(Qt::NoBrush);

            if (enabled) {
                // Draw outline shadows
                painter->setPen(option->palette.window().color().lighter(110));
                painter->drawLine(bounds.bottomLeft(), bounds.bottomRight());
                painter->drawLine(bounds.topRight(), bounds.bottomRight());
            }

            // Draw outline
            painter->setPen(Qt::black);
            bounds = option->rect;
            bounds.setRight(bounds.right() - 2);
            bounds.setBottom(bounds.bottom() - 2);
            painter->drawRect(bounds);

            // Paint the gradient
            QColor startColor;
            QColor midColor;
            QColor bandColor;
            QColor stopColor;

            if (enabled) {
                if (option->state & (State_Sunken | State_On)) {
                    startColor = option->palette.window().color().lighter(110);
                    midColor = option->palette.window().color().lighter(115);
                    bandColor = option->palette.window().color().darker(105);
                    stopColor = option->palette.window().color().darker(125);
                } else {
                    startColor = option->palette.window().color().lighter(100);
                    midColor = option->palette.window().color().lighter(105);
                    bandColor = option->palette.window().color().darker(110);
                    stopColor = option->palette.window().color().darker(135);
                }
            } else {
                startColor = option->palette.window().color().darker(150);
                midColor = option->palette.window().color().darker(155);
                bandColor = option->palette.window().color().darker(160);
                stopColor = option->palette.window().color().darker(170);
            }

            QRect gradientRect = bounds.adjusted(1, 1, 0, 0);
            QLinearGradient gradient(gradientRect.topRight(), gradientRect.bottomRight());

            gradient.setColorAt(0, startColor );
            gradient.setColorAt(0.5, midColor );
            gradient.setColorAt(0.51, bandColor );
            gradient.setColorAt(1, stopColor );
            painter->fillRect(gradientRect, gradient);

            if (enabled) {
                // Draw highlight
                QRect highlightRect = gradientRect.adjusted(0, 0, -1, -1);
                painter->setPen(option->palette.window().color().lighter(120));
                painter->drawLine(highlightRect.topLeft(), highlightRect.bottomLeft());
                painter->drawLine(highlightRect.topRight(), highlightRect.topLeft());
            }

            painter->restore();
        }
        break;


        case PE_IndicatorBranch:
        {
            // The FrameworkStyle does not draw TreeView indicator lines,
            // only the open and close icons.

            if (option->state & State_Children) {

                int drawX = 0;
                int drawY = 0;
                
                if (option->state & State_Open) {
                    drawX = (option->rect.width() - mBranchOpenIcon.width())  / 2;
                    drawY = (option->rect.height() - mBranchOpenIcon.height())  / 2;
                } else {
                    drawX = (option->rect.width() - mBranchClosedIcon.width())  / 2;
                    drawY = (option->rect.height() - mBranchClosedIcon.height())  / 2;
                }                    

                painter->drawPixmap(drawX, 
                                    drawY, 
                                    option->state & State_Open ? mBranchOpenIcon : mBranchClosedIcon);
            }
        }
        break;

        default:
            QCommonStyle::drawPrimitive(element, option, painter, widget);
            break;
    }            
}


//-----------------------------------------------------------------------------
// FrameworkStyle::drawControl()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::drawControl(ControlElement element, 
                         const QStyleOption* option, 
                         QPainter* painter, 
                         const QWidget* widget) const
{
    switch (element) {

        case CE_ColumnViewGrip:
        {
            painter->save();

            QPen pen(painter->pen());

            // Draw background
            painter->fillRect(QRect(0, 0, option->rect.width(), option->rect.height()),
                              option->palette.color(QPalette::Window));
            pen.setColor(option->palette.color(QPalette::Active, QPalette::Dark));
            painter->setPen(pen);
            painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
            painter->drawLine(option->rect.topRight(), option->rect.bottomRight());

            // Draw the grip area
            pen.setWidth(option->rect.width() / 20);
            pen.setColor(option->palette.color(QPalette::Active, QPalette::Dark));
            painter->setPen(pen);

            const int line1starting = (option->rect.width() * 8 / 20) - 1;
            const int line2starting = (option->rect.width() * 13 / 20) - 1;
            const int top = option->rect.height() * 20 / 75;
            const int bottom = option->rect.height() - 1 - top;

            painter->drawLine(line1starting, top, line1starting, bottom);
            painter->drawLine(line2starting, top, line2starting, bottom);

            pen.setColor(option->palette.color(QPalette::Active, QPalette::Light));
            painter->setPen(pen);
            painter->drawLine(line1starting + 1, top, line1starting + 1, bottom);
            painter->drawLine(line2starting + 1, top, line2starting + 1, bottom);

            painter->restore();
        }
        break;

        case CE_Header:
        {
            const QStyleOptionHeader* header = qstyleoption_cast<const QStyleOptionHeader*>(option);
            if (header != NULL) {
                QRegion clipRegion = painter->clipRegion();
                painter->setClipRect(option->rect);

                drawControl(CE_HeaderSection, header, painter, widget);
                
                QStyleOptionHeader subopt = *header;
                subopt.rect = subElementRect(SE_HeaderLabel, header, widget);
                
                if (subopt.rect.isValid())
                    drawControl(CE_HeaderLabel, &subopt, painter, widget);

                if (header->sortIndicator != QStyleOptionHeader::None) {
                    subopt.rect = subElementRect(SE_HeaderArrow, option, widget);
                    drawPrimitive(PE_IndicatorHeaderArrow, &subopt, painter, widget);
                }
                painter->setClipRegion(clipRegion);
            }
        }
        break;

        case CE_HeaderSection:
        {
            painter->save();

            // Get the rect to frame
            QRect frameRect = option->rect;

            // Fill the background
            QLinearGradient gradient(frameRect.topLeft(), frameRect.bottomLeft());
            gradient.setColorAt(0.0, option->palette.brush(QPalette::Window).color().lighter(115));
            gradient.setColorAt(1.0, option->palette.brush(QPalette::Window).color().darker(110));
            painter->fillRect(frameRect, QBrush(gradient));

            // Draw the highlight
            QPoint highLeft = frameRect.topLeft();
            QPoint highRight = frameRect.topLeft();
            highRight.setX(highRight.x() + 1);
            painter->setPen(option->palette.light().color().darker(150));
            painter->drawLine(highLeft, highRight);

            QPoint topLeft = frameRect.topLeft();
            painter->setPen(option->palette.light().color().darker(100));
            topLeft.setX(topLeft.x() + 2);
            painter->drawLine(topLeft, frameRect.topRight());

            // Draw the shadow
            painter->setPen(option->palette.dark().color().lighter(115));
            topLeft = frameRect.topLeft();
            topLeft.setY(topLeft.y() + 1);
            painter->drawLine(topLeft, frameRect.bottomLeft());
            painter->drawLine(frameRect.bottomLeft(), frameRect.bottomRight());

            // Draw the separators
            painter->setPen(option->palette.light().color());
            QPoint bottomRight = frameRect.bottomRight();
            bottomRight.setY(bottomRight.y() - 1);
            painter->drawLine(frameRect.topRight(), bottomRight);
                        
            painter->restore();
        }
        break;

        case CE_PushButton:
        {
            const QStyleOptionButton* btn = qstyleoption_cast<const QStyleOptionButton *>(option);
            if (btn != NULL) {
                proxy()->drawControl(CE_PushButtonBevel, btn, painter, widget);

                QStyleOptionButton subopt = *btn;
                subopt.rect = subElementRect(SE_PushButtonContents, btn, widget);
                proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);

                if (btn->state & State_HasFocus) {
                    QStyleOptionFocusRect fropt;
                    fropt.QStyleOption::operator=(*btn);
                    fropt.rect = subElementRect(SE_PushButtonFocusRect, btn, widget);
                    proxy()->drawPrimitive(PE_FrameFocusRect, &fropt, painter, widget);
                }
            }
        }
        break;

        case CE_PushButtonBevel:
        {
            const QStyleOptionButton* btn = qstyleoption_cast<const QStyleOptionButton *>(option);
            if (btn != NULL) {
                QRect br = btn->rect;
                int dbi = proxy()->pixelMetric(PM_ButtonDefaultIndicator, btn, widget);

                if (btn->features & QStyleOptionButton::DefaultButton)
                    proxy()->drawPrimitive(PE_FrameDefaultButton, option, painter, widget);

                if (btn->features & QStyleOptionButton::AutoDefaultButton)
                    br.setCoords(br.left() + dbi, br.top() + dbi, br.right() - dbi, br.bottom() - dbi);

                if (!(btn->features & (QStyleOptionButton::Flat | QStyleOptionButton::CommandLinkButton))
                    || btn->state & (State_Sunken | State_On)
                    || (btn->features & QStyleOptionButton::CommandLinkButton && btn->state & State_MouseOver)) {
                    QStyleOptionButton tmpBtn = *btn;
                    tmpBtn.rect = br;
                    proxy()->drawPrimitive(PE_PanelButtonCommand, &tmpBtn, painter, widget);
                }

                if (btn->features & QStyleOptionButton::HasMenu) {
                    int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, btn, widget);
                    QRect ir = btn->rect;
                    QStyleOptionButton newBtn = *btn;
                    newBtn.rect = QRect(ir.right() - mbi + 2, ir.height()/2 - mbi/2 + 3, mbi - 6, mbi - 6);
                    proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, painter, widget);
                }
            }
        }
        break;

        case CE_TabBarTab:
        {
            const QStyleOptionTab* tab = qstyleoption_cast<const QStyleOptionTab *>(option);
            if (tab != NULL) {
                proxy()->drawControl(CE_TabBarTabShape, tab, painter, widget);
                proxy()->drawControl(CE_TabBarTabLabel, tab, painter, widget);
            }
        }
        break;

#if 0
        case CE_TabBarTabShape:
        {
            const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option);
            if (tab != NULL) {

                if (tab->shape != QTabBar::RoundedNorth && tab->shape != QTabBar::RoundedWest &&
                    tab->shape != QTabBar::RoundedSouth && tab->shape != QTabBar::RoundedEast) {
                    QWindowsStyle::drawControl(element, option, painter, widget);
                    break;
                }

                painter->save();

                // Set up some convenience variables
                bool disabled = !(tab->state & State_Enabled);
                bool onlyTab = tab->position == QStyleOptionTab::OnlyOneTab;
                bool selected = tab->state & State_Selected;
                bool mouseOver = (tab->state & State_MouseOver) && !selected && !disabled;
                bool previousSelected = tab->selectedPosition == QStyleOptionTab::PreviousIsSelected;
                bool nextSelected = tab->selectedPosition == QStyleOptionTab::NextIsSelected;
                bool leftCornerWidget = (tab->cornerWidgets & QStyleOptionTab::LeftCornerWidget);
                bool reverse = (tab->direction == Qt::RightToLeft);

                int lowerTop = selected ? 0 : 3; // to make the selected tab bigger than the rest
                QRect adjustedRect;
                bool atEnd = (tab->position == QStyleOptionTab::End) || onlyTab;
                bool atBeginning = ((tab->position == QStyleOptionTab::Beginning) || onlyTab)
                                   && !leftCornerWidget;
                bool reverseShadow = false;

                int borderThickness = proxy()->pixelMetric(PM_TabBarBaseOverlap, tab, widget);
                int marginLeft = 0;
                if ((atBeginning && !selected) || (selected && leftCornerWidget && ((tab->position == QStyleOptionTab::Beginning) || onlyTab))) {
                    marginLeft = 1;
                }

                // I've set the names based on the natural coordinate system. Vectors are used to rotate everything
                // if the orientation of the tab bare is different than north.
                {
                    // Coordinates of corners of rectangle for transformation
                    QPoint topLeft;
                    QPoint topRight;
                    QPoint bottomLeft;
                    QPoint bottomRight;

                    // Fill with normalized vectors in the direction of the coordinate system
                    // (down and right should be complement of up and left, or it will look odd)
                    QPoint vectorUp;
                    QPoint vectorDown;
                    QPoint vectorLeft;
                    QPoint vectorRight;

                    QBrush border = option->palette.shadow();
                    //qBrushSetAlphaF(&border, qreal(0.4));
                    QBrush innerTopLeft = option->palette.shadow();
                    //qBrushSetAlphaF(&innerTopLeft, qreal(0.075));
                    QBrush innerBottomRight = option->palette.shadow();
                    //qBrushSetAlphaF(&innerBottomRight, qreal(0.23));
                    QBrush corner = option->palette.shadow();
                    //qBrushSetAlphaF(&corner, qreal(0.25));

                    QBrush baseColor1;
                    QBrush baseColor2;

                    switch (tab->shape) {
                    case QTabBar::RoundedNorth:
                        vectorUp = QPoint(0, -1);
                        vectorDown = QPoint(0, 1);

                        if (reverse) {
                            vectorLeft = QPoint(1, 0);
                            vectorRight = QPoint(-1, 0);
                            reverseShadow = true;
                        } else {
                            vectorLeft = QPoint(-1, 0);
                            vectorRight = QPoint(1, 0);
                        }

                        if (reverse) {
                            topLeft = tab->rect.topRight();
                            topRight = tab->rect.topLeft();
                            bottomLeft = tab->rect.bottomRight();
                            bottomRight = tab->rect.bottomLeft();
                        } else {
                            topLeft = tab->rect.topLeft();
                            topRight = tab->rect.topRight();
                            bottomLeft = tab->rect.bottomLeft();
                            bottomRight = tab->rect.bottomRight();
                        }


                        baseColor1 = border;
                        baseColor2 = innerTopLeft;
                        break ;
                    case QTabBar::RoundedWest:
                        vectorUp = QPoint(-1, 0);
                        vectorDown = QPoint(1, 0);
                        vectorLeft = QPoint(0, -1);
                        vectorRight = QPoint(0, 1);

                        topLeft = tab->rect.topLeft();
                        topRight = tab->rect.bottomLeft();
                        bottomLeft = tab->rect.topRight();
                        bottomRight = tab->rect.bottomRight();

                        baseColor1 = border;
                        baseColor2 = innerTopLeft;
                        break ;
                    case QTabBar::RoundedEast:
                        vectorUp = QPoint(1, 0);
                        vectorDown = QPoint(-1, 0);
                        vectorLeft = QPoint(0, -1);
                        vectorRight = QPoint(0, 1);

                        topLeft = tab->rect.topRight();
                        topRight = tab->rect.bottomRight();
                        bottomLeft = tab->rect.topLeft();
                        bottomRight = tab->rect.bottomLeft();

                        baseColor1 = border;
                        baseColor2 = innerBottomRight;
                        break ;
                    case QTabBar::RoundedSouth:
                        vectorUp = QPoint(0, 1);
                        vectorDown = QPoint(0, -1);

                        if (reverse) {
                            vectorLeft = QPoint(1, 0);
                            vectorRight = QPoint(-1, 0);
                            reverseShadow = true;

                            topLeft = tab->rect.bottomRight();
                            topRight = tab->rect.bottomLeft();
                            bottomLeft = tab->rect.topRight();
                            bottomRight = tab->rect.topLeft();
                        } else {
                            vectorLeft = QPoint(-1, 0);
                            vectorRight = QPoint(1, 0);

                            topLeft = tab->rect.bottomLeft();
                            topRight = tab->rect.bottomRight();
                            bottomLeft = tab->rect.topLeft();
                            bottomRight = tab->rect.topRight();
                        }

                        baseColor1 = border;
                        baseColor2 = innerBottomRight;
                        break ;
                    default:
                        break;
                    }

                    // Make the tab smaller when it's at the end, so that we are able to draw the corner
                    if (atEnd) {
                        topRight += vectorLeft;
                        bottomRight += vectorLeft;
                    }

                    {
                        // Outer border
                        QLine topLine;
                        {
                            QPoint adjustTopLineLeft = (vectorRight * (marginLeft + (previousSelected ? 0 : 1))) +
                                                       (vectorDown * lowerTop);
                            QPoint adjustTopLineRight = (vectorDown * lowerTop);
                            if (atBeginning || selected)
                                adjustTopLineLeft += vectorRight;
                            if (atEnd || selected)
                                adjustTopLineRight += 2 * vectorLeft;

                            topLine = QLine(topLeft + adjustTopLineLeft, topRight + adjustTopLineRight);
                        }

                        QLine leftLine;
                        {
                            QPoint adjustLeftLineTop = (vectorRight * marginLeft) + (vectorDown * (lowerTop + 1));
                            QPoint adjustLeftLineBottom = (vectorRight * marginLeft) + (vectorUp * borderThickness);
                            if (atBeginning || selected)
                                adjustLeftLineTop += vectorDown; // Make place for rounded corner
                            if (atBeginning && selected)
                                adjustLeftLineBottom += borderThickness * vectorDown;
                            else if (selected)
                                adjustLeftLineBottom += vectorUp;

                            leftLine = QLine(topLeft + adjustLeftLineTop, bottomLeft + adjustLeftLineBottom);
                        }

                        QLine rightLine;
                        {
                            QPoint adjustRightLineTop = vectorDown * (2 + lowerTop);
                            QPoint adjustRightLineBottom = vectorUp * borderThickness;
                            if (selected)
                                adjustRightLineBottom += vectorUp;

                            rightLine = QLine(topRight + adjustRightLineTop, bottomRight + adjustRightLineBottom);
                        }

                        // Background
                        QPoint startPoint = topLine.p1() + vectorDown + vectorLeft;
                        if (mouseOver)
                            startPoint += vectorDown;
                        QPoint endPoint = rightLine.p2();

                        if (tab->state & State_Enabled) {
                            QRect fillRect = QRect(startPoint, endPoint).normalized();
                            if (fillRect.isValid()) {
                                if (selected) {
                                    fillRect = QRect(startPoint, endPoint + vectorLeft + vectorDown * 3).normalized();
                                    painter->fillRect(fillRect, option->palette.window());

                                    // Connect to the base
                                    painter->setPen(QPen(option->palette.window(), 0));
                                    QVarLengthArray<QPoint, 6> points;
                                    points.append(rightLine.p2() + vectorDown);
                                    points.append(rightLine.p2() + vectorDown + vectorDown);
                                    points.append(rightLine.p2() + vectorDown + vectorDown + vectorRight);
                                    if (tab->position != QStyleOptionTab::Beginning) {
                                        points.append(leftLine.p2() + vectorDown);
                                        points.append(leftLine.p2() + vectorDown + vectorDown);
                                        points.append(leftLine.p2() + vectorDown + vectorDown + vectorLeft);
                                    }
                                    painter->drawPoints(points.constData(), points.size());
                                } else {
                                    QBrush buttonGradientBrush;
                                    QBrush buttonBrush = qMapBrushToRect(option->palette.button(), fillRect);
                                    if (buttonBrush.gradient() || !buttonBrush.texture().isNull()) {
                                        buttonGradientBrush = buttonBrush;
                                    } else {
                                        // Generate gradients
                                        QLinearGradient buttonGradient(fillRect.topLeft(), fillRect.bottomLeft());
                                        buttonGradient.setColorAt(0.0, buttonBrush.color().lighter(104));
                                        buttonGradient.setColorAt(1.0, buttonBrush.color().darker(110));
                                        buttonGradientBrush = QBrush(buttonGradient);
                                    }

                                    painter->fillRect(fillRect, buttonGradientBrush);
                                }
                            }
                        }

                        QPoint rightCornerDot = topRight + vectorLeft + (lowerTop + 1)*vectorDown;
                        QPoint leftCornerDot = topLeft + (marginLeft + 1)*vectorRight + (lowerTop + 1)*vectorDown;
                        QPoint bottomRightConnectToBase = rightLine.p2() + vectorRight + vectorDown;
                        QPoint bottomLeftConnectToBase = leftLine.p2() + vectorLeft + vectorDown;

                        painter->setPen(QPen(border, 0));

                        QVarLengthArray<QLine, 3> lines;
                        QVarLengthArray<QPoint, 7> points;

                        lines.append(topLine);

                        if (mouseOver) {
                            painter->drawLines(lines.constData(), lines.count());
                            lines.clear();

                            QLine secondHoverLine = QLine(topLine.p1() + vectorDown * 2 + vectorLeft, topLine.p2() + vectorDown * 2 + vectorRight);
                            painter->setPen(highlightedLightInnerBorderColor);
                            painter->drawLine(secondHoverLine);
                        }

                        if (mouseOver)
                            painter->setPen(QPen(border, 0));

                        if (!previousSelected)
                            lines.append(leftLine);
                        if (atEnd || selected) {
                            lines.append(rightLine);
                            points.append(rightCornerDot);
                        }
                        if (atBeginning || selected)
                            points.append(leftCornerDot);
                        if (selected) {
                            points.append(bottomRightConnectToBase);
                            points.append(bottomLeftConnectToBase);
                        }
                        if (lines.size() > 0) {
                            painter->drawLines(lines.constData(), lines.size());
                            lines.clear();
                        }
                        if (points.size() > 0) {
                            painter->drawPoints(points.constData(), points.size());
                            points.clear();
                        }

                        // Antialiasing
                        painter->setPen(QPen(corner, 0));
                        if (atBeginning || selected)
                            points.append(topLine.p1() + vectorLeft);
                        if (!previousSelected)
                            points.append(leftLine.p1() + vectorUp);
                        if (atEnd || selected) {
                            points.append(topLine.p2() + vectorRight);
                            points.append(rightLine.p1() + vectorUp);
                        }

                        if (selected) {
                            points.append(bottomRightConnectToBase + vectorLeft);
                            if (!atBeginning) {
                                points.append(bottomLeftConnectToBase + vectorRight);

                                if (((tab->position == QStyleOptionTab::Beginning) || onlyTab) && leftCornerWidget) {
                                    // A special case: When the first tab is selected and
                                    // has a left corner widget, it needs to do more work
                                    // to connect to the base
                                    QPoint p1 = bottomLeftConnectToBase + vectorDown;

                                    points.append(p1);
                                }
                            }
                        }
                        if (points.size() > 0) {
                            painter->drawPoints(points.constData(), points.size());
                            points.clear();
                        }

                        // Inner border
                        QLine innerTopLine = QLine(topLine.p1() + vectorDown, topLine.p2() + vectorDown);
                        if (!selected) {
                            QLinearGradient topLineGradient(innerTopLine.p1(),innerTopLine.p2());
                            topLineGradient.setColorAt(0, lightShadowGradientStartColor);
                            topLineGradient.setColorAt(1, lightShadowGradientStopColor);
                            painter->setPen(QPen(mouseOver ? QBrush(highlightedDarkInnerBorderColor) : QBrush(topLineGradient), 1));
                        } else {
                            painter->setPen(QPen(innerTopLeft, 0));
                        }
                        painter->drawLine(innerTopLine);

                        QLine innerLeftLine = QLine(leftLine.p1() + vectorRight + vectorDown, leftLine.p2() + vectorRight);
                        QLine innerRightLine = QLine(rightLine.p1() + vectorLeft + vectorDown, rightLine.p2() + vectorLeft);

                        if (selected) {
                            innerRightLine = QLine(innerRightLine.p1() + vectorUp, innerRightLine.p2());
                            innerLeftLine = QLine(innerLeftLine.p1() + vectorUp, innerLeftLine.p2());
                        }

                        if (selected || atBeginning) {
                            QBrush leftLineGradientBrush;
                            QRect rect = QRect(innerLeftLine.p1(), innerLeftLine.p2()).normalized();
                            QBrush buttonBrush = qMapBrushToRect(option->palette.button(), rect);
                            if (buttonBrush.gradient() || !buttonBrush.texture().isNull()) {
                                leftLineGradientBrush = qBrushLight(buttonBrush, 105);
                            } else {
                                QLinearGradient buttonGradient3(rect.topLeft(), rect.bottomLeft());
                                buttonGradient3.setColorAt(0.0, buttonBrush.color().lighter(105));
                                buttonGradient3.setColorAt(1.0, buttonBrush.color());
                                leftLineGradientBrush = QBrush(buttonGradient3);
                            }

                            if (!selected)
                                painter->setPen(QPen(leftLineGradientBrush, 0));

                            // Assume the sun is on the same side in Right-To-Left layouts and draw the
                            // light shadow on the left side always (the right line is on the left side in
                            // reverse layouts for north and south)
                            if (reverseShadow)
                                painter->drawLine(innerRightLine);
                            else
                                painter->drawLine(innerLeftLine);
                        }

                        if (atEnd || selected) {
                            if (!selected) {
                                QBrush rightLineGradientBrush;
                                QRect rect = QRect(innerRightLine.p1(), innerRightLine.p2()).normalized();
                                QBrush buttonBrush = qMapBrushToRect(option->palette.button(), rect);
                                if (buttonBrush.gradient() || !buttonBrush.texture().isNull()) {
                                    rightLineGradientBrush = qBrushDark(buttonBrush, 105);
                                } else {
                                    QLinearGradient buttonGradient4(rect.topLeft(), rect.bottomLeft());
                                    buttonGradient4.setColorAt(0.0, buttonBrush.color());
                                    buttonGradient4.setColorAt(1.0, buttonBrush.color().darker(110));
                                    rightLineGradientBrush = QBrush(buttonGradient4);
                                }

                                painter->setPen(QPen(rightLineGradientBrush, 0));
                            } else {
                                painter->setPen(QPen(innerBottomRight, 0));
                            }

                            if (reverseShadow)
                                painter->drawLine(innerLeftLine);
                            else
                                painter->drawLine(innerRightLine);
                        }


                        // Base
                        QLine baseLine = QLine(bottomLeft + marginLeft * 2 * vectorRight, bottomRight);
                        {

                            QPoint adjustedLeft;
                            QPoint adjustedRight;

                            if (atEnd && !selected) {
                                baseLine = QLine(baseLine.p1(), baseLine.p2() + vectorRight);
                            }

                            if (nextSelected) {
                                adjustedRight += vectorLeft;
                                baseLine = QLine(baseLine.p1(), baseLine.p2() + vectorLeft);
                            }
                            if (previousSelected) {
                                adjustedLeft += vectorRight;
                                baseLine = QLine(baseLine.p1() + vectorRight, baseLine.p2());
                            }
                            if (atBeginning)
                                adjustedLeft += vectorRight;

                            painter->setPen(QPen(baseColor2, 0));
                            if (!selected)
                                painter->drawLine(baseLine);

                            if (atEnd && !selected)
                                painter->drawPoint(baseLine.p2() + vectorRight);

                            if (atBeginning && !selected)
                                adjustedLeft = vectorRight;
                            else
                                adjustedLeft = QPoint(0, 0);
                            painter->setPen(QPen(baseColor1, 0));
                            if (!selected)
                                painter->drawLine(bottomLeft + vectorUp + adjustedLeft, baseLine.p2() + vectorUp);

                            QPoint endPoint = bottomRight + vectorUp;
                            if (atEnd && !selected)
                                painter->drawPoint(endPoint);

                            // For drawing a lower left "fake" corner on the base when the first tab is unselected
                            if (atBeginning && !selected) {
                                painter->drawPoint(baseLine.p1() + vectorLeft);
                            }

                            painter->setPen(QPen(corner, 0));
                            if (nextSelected)
                                painter->drawPoint(endPoint);
                            else if (selected)
                                painter->drawPoint(endPoint + vectorRight);

                            // For drawing a lower left "fake" corner on the base when the first tab is unselected
                            if (atBeginning && !selected) {
                                painter->drawPoint(baseLine.p1() + 2 * vectorLeft);
                            }
                        }
                    }
                }

            painter->restore();
        }
        break;
#endif

        default:
            QCommonStyle::drawControl(element, option, painter, widget);
            break;
    }        
}


//-----------------------------------------------------------------------------
// FrameworkStyle::drawComplexControl()
//-----------------------------------------------------------------------------
void 
FrameworkStyle::drawComplexControl(ComplexControl control, 
                                const QStyleOptionComplex* option, 
                                QPainter* painter, 
                                const QWidget* widget) const
{
    QCommonStyle::drawComplexControl(control, option, painter, widget);
}

//-----------------------------------------------------------------------------
// FrameworkStyle::drawItemText()
//-----------------------------------------------------------------------------
void
FrameworkStyle::drawItemText(QPainter *painter,
                          const QRect &rectangle,
                          int alignment,
                          const QPalette &palette,
                          bool enabled,
                          const QString &text,
                          QPalette::ColorRole textRole) const
{
    QCommonStyle::drawItemText(painter, rectangle, alignment, palette,
                                   enabled, text, textRole);
}

//-----------------------------------------------------------------------------
// FrameworkStyle::generatedIconPixmap()
//-----------------------------------------------------------------------------
QPixmap 
FrameworkStyle::generatedIconPixmap(QIcon::Mode iconMode, 
                                 const QPixmap& pixmap, 
                                 const QStyleOption* option) const
{
    return QCommonStyle::generatedIconPixmap(iconMode, pixmap, option);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::hitTestComplexControl()
//-----------------------------------------------------------------------------
QStyle::SubControl 
FrameworkStyle::hitTestComplexControl(ComplexControl control, 
                                   const QStyleOptionComplex* option, 
                                   const QPoint& position, 
                                   const QWidget* widget ) const
{
    return QCommonStyle::hitTestComplexControl(control, option, position, widget);

}                                   


//-----------------------------------------------------------------------------
// FrameworkStyle::pixelMetric()
//-----------------------------------------------------------------------------
int 
FrameworkStyle::pixelMetric(PixelMetric metric, 
                         const QStyleOption* option, 
                         const QWidget* widget) const
{
    int ret = QCommonStyle::pixelMetric(metric, option, widget);

    switch (metric) {
        case PM_HeaderMargin:
            ret = 2;
            break;

        default:
            break;
    }  

    return ret;                              
}


//-----------------------------------------------------------------------------
// FrameworkStyle::sizeFromContents()
//-----------------------------------------------------------------------------
QSize 
FrameworkStyle::sizeFromContents(ContentsType contentsType, 
                              const QStyleOption* option, 
                              const QSize& contentsSize, 
                              const QWidget* widget) const
{
    QSize sz = QCommonStyle::sizeFromContents(contentsType, option, contentsSize, widget);

    switch (contentsType) {

        case CT_HeaderSection:
        {
            const QStyleOptionHeader* hdr = qstyleoption_cast<const QStyleOptionHeader *>(option);
            if (hdr != NULL) {
                const bool nullIcon = hdr->icon.isNull();
                const int margin = pixelMetric(QStyle::PM_HeaderMargin, hdr, widget);
                const int iconSize = nullIcon ? 0 : pixelMetric(QStyle::PM_SmallIconSize, hdr, widget);
                QSize txt = hdr->fontMetrics.size(0, hdr->text);

                sz = contentsSize;
                sz.setHeight(margin + qMax(iconSize, txt.height()) + margin);
                sz.setWidth((nullIcon ? 0 : margin) + iconSize
                            + (hdr->text.isNull() ? 0 : margin) + txt.width() + margin);
            }
        }
        break;

        default:
            break;
    }

    return sz;
}


//-----------------------------------------------------------------------------
// FrameworkStyle::standardIconImplementation()
//-----------------------------------------------------------------------------
QIcon
FrameworkStyle::standardIconImplementation(StandardPixmap standardIcon,
                                        const QStyleOption *option,
                                        const QWidget *widget) const
{
    return standardIconImplementation(standardIcon, option, widget);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::styleHint()
//-----------------------------------------------------------------------------
// Override any style hints we don't like from the base class
int 
FrameworkStyle::styleHint(StyleHint hint, 
                       const QStyleOption* option, 
                       const QWidget* widget, 
                       QStyleHintReturn* returnData) const
{
    int result = 0;
    
    switch (hint) {
        case SH_EtchDisabledText:
            // TODO djohnson temporary until we fix drawing etched text for dark palettes
            result = 0;
            break;

        case SH_Table_GridLineColor:
        {
            if (option != NULL)
                result = option->palette.color(QPalette::Mid).rgb();
            else
                result = -1;
            break;
        }
        
        default:
            result = QCommonStyle::styleHint(hint, option, widget, returnData);
    }

    return result;
}


//-----------------------------------------------------------------------------
// FrameworkStyle::subControlRect()
//-----------------------------------------------------------------------------
QRect 
FrameworkStyle::subControlRect(ComplexControl control, 
                            const QStyleOptionComplex* option, 
                            SubControl subControl, 
                            const QWidget* widget) const
{
    return QCommonStyle::subControlRect(control, option, subControl, widget);
}


//-----------------------------------------------------------------------------
// FrameworkStyle::subElementRect()
//-----------------------------------------------------------------------------
QRect 
FrameworkStyle::subElementRect(SubElement element, 
                            const QStyleOption* option, 
                            const QWidget* widget) const
{
    return QCommonStyle::subElementRect(element, option, widget);
}


