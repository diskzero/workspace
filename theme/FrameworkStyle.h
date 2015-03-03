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

#ifndef FRAMEWORKSTYLE_HAS_BEEN_INCLUDED
#define FRAMEWORKSTYLE_HAS_BEEN_INCLUDED

// Qt
#include <QCommonStyle>
#include <QPixmap>

namespace workspace {

//=============================================================================
// class FrameworkStyle
//=============================================================================
/*! Provides the standard studio style.
    Draw controls and text using the standard, studio
    approved, style.
*/
class FrameworkStyle : public QCommonStyle
{
    Q_OBJECT

public:
    /// Default constructor
    FrameworkStyle();
    /// Destructor
    virtual ~FrameworkStyle();

    /// Draw the given control using the provided painter and options
    virtual void drawComplexControl(ComplexControl control,
                                    const QStyleOptionComplex* option,
                                    QPainter* painter,
                                    const QWidget* widget=NULL) const;
    /// Draw the given element with the provided painter and options
    virtual void drawControl(ControlElement element,
                             const QStyleOption* option,
                             QPainter* painter,
                             const QWidget* widget=NULL) const;
    /// Draw teh given element with the provided painter and options
    virtual void drawPrimitive(PrimitiveElement element,
                               const QStyleOption* option,
                               QPainter* painter,
                               const QWidget* widget=NULL) const;

    /// Draw the given text using the provided painter and palette
    virtual void drawItemText(QPainter *painter,
                              const QRect &rectangle,
                              int alignment,
                              const QPalette &palette,
                              bool enabled,
                              const QString &text,
                              QPalette::ColorRole textRole=QPalette::NoRole) const;

    /// Return a copy of the given pixmap, conforming to the provided mode and options
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode,
                                        const QPixmap& pixmap,
                                        const QStyleOption* option) const;
    /// Return the sub control at the given position in the given complex control
    virtual SubControl hitTestComplexControl(ComplexControl control,
                                             const QStyleOptionComplex* option,
                                             const QPoint& position,
                                             const QWidget* widget = NULL) const;
    /// Return the value of the given metric
    virtual int pixelMetric(PixelMetric metric,
                            const QStyleOption* option=NULL,
                            const QWidget* widget=NULL) const;

    /// Adjusts palette to conform to style requirements (if any)
    virtual void polish(QPalette& palette);
    /// Late initialization of the application appearance
    virtual void polish(QApplication* application);
    /// Initialize the appearance of the given widget
    virtual void polish(QWidget* widget);
    /// Uninitialize the widget's appearance
    virtual void unpolish(QWidget* widget);
    /// Uninitialize the application appearance
    virtual void unpolish(QApplication* application);

    /// Return the size of the element described by the specified option and type
    virtual QSize sizeFromContents(ContentsType contentsType,
                                   const QStyleOption* option,
                                   const QSize& contentsSize,
                                   const QWidget* widget=NULL) const;

    /// Return an icon for the given standardIcon
    virtual QIcon standardIconImplementation(StandardPixmap standardIcon,
                                             const QStyleOption *option=NULL,
                                             const QWidget *widget=NULL) const;
    /// Return the style's standard palette
    virtual QPalette standardPalette() const;

    /// Return the rectangle containing the specified subcontrol
    virtual QRect subControlRect(ComplexControl control,
                                 const QStyleOptionComplex* option,
                                 SubControl subControl,
                                 const QWidget* widget=NULL) const;

    /// Return the sub-area for the given element as described in the provided option
    virtual QRect subElementRect(SubElement element,
                                 const QStyleOption* option,
                                 const QWidget* widget=NULL) const;

    /// Return the specified style hint using given widget and option
    virtual int styleHint(StyleHint hint,
                          const QStyleOption* option=NULL,
                          const QWidget* widget=NULL,
                          QStyleHintReturn* returnData=NULL) const;

    /// Generate an appropriate palette from the given color and contrast
    QPalette generatePalette(int hue, int sat, int val, int contrast=50) const;

private:
    QPixmap mBranchOpenIcon;
    QPixmap mBranchClosedIcon;  
};

} // namespace workspace


#endif // FRAMEWORKSTYLE_HAS_BEEN_INCLUDED
