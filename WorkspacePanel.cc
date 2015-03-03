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
#include "WorkspacePanel.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QCursor>
#include <QEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QToolButton>
#include <QtDebug>

// Needed for window title drag key modifier detection
#ifndef Q_WS_MAC
#include <QX11Info>
#include <X11/Xlib.h>
#else
#include <Carbon.h>
#endif

// Local
#include "WorkspaceArea.h"
#include "WorkspaceLayout.h"
#include "WorkspaceTabBar.h"
#include "WorkspaceUtils.h"

namespace {

QString
qt_setWindowTitle_helperHelper(const QString &title, const QWidget *widget)
{
    Q_ASSERT(widget);

#ifdef QT_EVAL
    extern QString qt_eval_adapt_window_title(const QString &title);
    QString cap = qt_eval_adapt_window_title(title);
#else
    QString cap = title;
#endif

    if (cap.isEmpty())
        return cap;

    QLatin1String placeHolder("[*]");
    int placeHolderLength = 3; // QLatin1String doesn't have length()

    int index = cap.indexOf(placeHolder);

    // here the magic begins
    while (index != -1) {
        index += placeHolderLength;
        int count = 1;
        while (cap.indexOf(placeHolder, index) == index) {
            ++count;
            index += placeHolderLength;
        }

        if (count%2) { // odd number of [*] -> replace last one
            int lastIndex = cap.lastIndexOf(placeHolder, index - 1);
            if (widget->isWindowModified()
             && widget->style()->styleHint(QStyle::SH_TitleBar_ModifyNotification, 0, widget))
                cap.replace(lastIndex, 3, QWidget::tr("*"));
            else
                cap.remove(lastIndex, 3);
        }

        index = cap.indexOf(placeHolder, index);
    }

    cap.replace(QLatin1String("[*][*]"), placeHolder);

    return cap;
}

}


//-----------------------------------------------------------------------------
// translateModifiers
// 
/// Translate X11 modifier to Qt modifier
/// \result The translated modifier
/// \param inState The X11 state.
//-----------------------------------------------------------------------------
#ifdef Q_WS_MAC
static Qt::KeyboardModifiers 
translateModifiers(UInt32 inState)
{
    bool shiftKey, ctrlKey, altKey, metaKey;
    shiftKey = inState & ::shiftKey;
    ctrlKey = inState & ::controlKey;
    altKey = inState & ::optionKey;
    metaKey = inState & ::cmdKey;

#if 0    
    qDebug() << shiftKey;
    qDebug() << ctrlKey;
    qDebug() << altKey;
    qDebug() << metaKey;
    qDebug() << "\n\n";
#endif
    
    Qt::KeyboardModifiers result = 0;
    
    if (shiftKey)
        result |= Qt::ShiftModifier;
        
    if (ctrlKey)
        result |= Qt::ControlModifier;

    if (altKey)
        result |= Qt::AltModifier;

    if (metaKey)
        result |= Qt::MetaModifier;

    return result;
}
#else
static Qt::KeyboardModifiers 
translateModifiers(int inState)
{
    Qt::KeyboardModifiers result = 0;
    
    if (inState & ShiftMask)
        result |= Qt::ShiftModifier;
        
    if (inState & ControlMask)
        result |= Qt::ControlModifier;

    if (inState & Mod1Mask)
        result |= Qt::AltModifier;

    if (inState & Mod2Mask)
        result |= Qt::MetaModifier;

    return result;
}
#endif


//-----------------------------------------------------------------------------
// getCurrentKeyboardModifiers
// 
/// Utility to get keyboard state outside of Qt event loop.
/// \result The current keyboard modifiers
//-----------------------------------------------------------------------------
static Qt::KeyboardModifiers 
getCurrentKeyboardModifiers()
{
#ifndef Q_WS_MAC    
    Window root;
    Window child;
    int root_x, root_y, win_x, win_y;
    uint keyState;

    for (int index = 0; index < ScreenCount(QX11Info::display()); ++index) {
        if (XQueryPointer(QX11Info::display(), 
                          QX11Info::appRootWindow(index), 
                          &root, 
                          &child, 
                          &root_x, 
                          &root_y, 
                          &win_x, 
                          &win_y, 
                          &keyState)) {
            return translateModifiers(keyState & 0x00ff);
        }
    }
#else
    return translateModifiers(::GetCurrentKeyModifiers());
#endif

    return 0;
}


#ifndef Q_WS_MAC
//-----------------------------------------------------------------------------
// isMouseButtonDown()
// 
/// Utility to get mouse button state outside of Qt event loop.
//-----------------------------------------------------------------------------
static bool
isMouseButtonDown(unsigned int inMask)
{
    ::Window root_return;
    :: Window child_return;
    int root_x_return, root_y_return, win_x_return, win_y_return;
    unsigned int mask_return = 0;

    for (int index = 0; index < ScreenCount(QX11Info::display()); ++index) {

        if (XQueryPointer(QX11Info::display(), 
    		              QX11Info::appRootWindow(index),
    		              &root_return,
    		              &child_return,
    		              &root_x_return,
    		              &root_y_return,
    		              &win_x_return,
    		              &win_y_return,
    		              &mask_return)) {

            return mask_return & inMask;
        }
    }
    return false;
}
#endif


//-----------------------------------------------------------------------------
// globalMousePos()
// 
/// Utility to get mouse position outside of Qt event loop.
//-----------------------------------------------------------------------------
static QPoint
globalMousePos()
{
#ifndef Q_WS_MAC
    ::Window root_return;
    :: Window child_return;
    int root_x_return, root_y_return, win_x_return, win_y_return;
    unsigned int mask_return = 0;

    for (int index = 0; index < ScreenCount(QX11Info::display()); ++index) {

        if (XQueryPointer(QX11Info::display(), 
    		              QX11Info::appRootWindow(index),
    		              &root_return,
    		              &child_return,
    		              &root_x_return,
    		              &root_y_return,
    		              &win_x_return,
    		              &win_y_return,
    		              &mask_return)) {

            return QPoint(root_x_return, root_y_return);
        }
    }
#endif
    
    return QPoint();
}


//=============================================================================
// class WorkspacePanelLayout
//=============================================================================

WorkspacePanelLayout::WorkspacePanelLayout(QWidget* inParent)
    :	QLayout(inParent)
    ,	mItemList(RoleCount, 0)
{
}


WorkspacePanelLayout::~WorkspacePanelLayout()
{
    qDeleteAll(mItemList);
}

void
WorkspacePanelLayout::addItem(QLayoutItem*)
{
    qWarning() << "WorkspacePanelLayout::addItem(): please use WorkspacePanelLayout::setWidget()";
    return;
}


QLayoutItem*
WorkspacePanelLayout::itemAt(int index) const
{
    int cnt = 0;
    for (int i = 0; i < mItemList.count(); ++i) {
        QLayoutItem *item = mItemList.at(i);
        if (item == NULL)
            continue;
        if (index == cnt++)
            return item;
    }
    return NULL;
}


QLayoutItem*
WorkspacePanelLayout::takeAt(int index)
{
    int j = 0;
    for (int i = 0; i < mItemList.count(); ++i) {
        QLayoutItem *item = mItemList.at(i);
        if (item == NULL)
            continue;
            
        if (index == j) {
            mItemList[i] = 0;
            invalidate();
            return item;
        }
        ++j;
    }
    
    return NULL;
}


int
WorkspacePanelLayout::count() const
{
    int result = 0;
    for (int i = 0; i < mItemList.count(); ++i) {
        if (mItemList.at(i))
            ++result;
    }
    
    return result;
}


QSize
WorkspacePanelLayout::sizeFromContent(const QSize &content) const
{
    QSize result = content;

    result.setHeight(qMax(result.height(), 0));
    result.setWidth(qMax(content.width(), minimumTitleWidth()));

    WorkspacePanel* w = qobject_cast<WorkspacePanel*>(parentWidget());

    const int th = titleHeight();
    result += QSize(0, th);

    result.setHeight(qMin(result.height(), (int) QWIDGETSIZE_MAX));
    result.setWidth(qMin(result.width(), (int) QWIDGETSIZE_MAX));

    if (content.width() < 0)
        result.setWidth(-1);
    if (content.height() < 0)
        result.setHeight(-1);

    int left, top, right, bottom;
    w->getContentsMargins(&left, &top, &right, &bottom);
    //we need to substract the contents margin (it will be added by the caller)
    QSize min = w->minimumSize() - QSize(left + right, top + bottom);
    QSize max = w->maximumSize() - QSize(left + right, top + bottom);

    /* A floating dockwidget will automatically get its minimumSize set to the layout's
       minimum size + deco. We're *not* interested in this, we only take minimumSize()
       into account if the user set it herself. Otherwise we end up expanding the result
       of a calculation for a non-floating dock widget to a floating dock widget's
       minimum size + window decorations. */

    uint explicitMin = 0;
    uint explicitMax = 0;
    // TODO mrequenes Ack! No access to d_func
#if 0
    if (w->d_func()->extra != NULL) {
        explicitMin = w->d_func()->extra->explicitMinSize;
        explicitMax = w->d_func()->extra->explicitMaxSize;
    }
#endif

    if (!(explicitMin & Qt::Horizontal) || min.width() == 0)
        min.setWidth(-1);
    if (!(explicitMin & Qt::Vertical) || min.height() == 0)
        min.setHeight(-1);

    if (!(explicitMax & Qt::Horizontal))
        max.setWidth(QWIDGETSIZE_MAX);
    if (!(explicitMax & Qt::Vertical))
        max.setHeight(QWIDGETSIZE_MAX);

    return result.boundedTo(max).expandedTo(min);
}


QSize
WorkspacePanelLayout::sizeHint() const
{
    QSize content(-1, -1);
    if (mItemList[Content] != NULL)
        content = mItemList[Content]->sizeHint();

    return sizeFromContent(content);
}

QSize
WorkspacePanelLayout::maximumSize() const
{
    if (mItemList[Content] != NULL) {
        const QSize content = mItemList[Content]->maximumSize();
        return sizeFromContent(content);
    } else {
        return parentWidget()->maximumSize();
    }

}

QSize
WorkspacePanelLayout::minimumSize() const
{
    QSize content(0, 0);
    if (mItemList[Content] != NULL)
        content = mItemList[Content]->minimumSize();

    return sizeFromContent(content);
}

QWidget*
WorkspacePanelLayout::widgetForRole(Role r) const
{
    QLayoutItem *item = mItemList.at(r);
    return item == NULL ? NULL : item->widget();
}

void
WorkspacePanelLayout::setWidgetForRole(Role r, QWidget* w)
{
    QWidget* old = widgetForRole(r);
    if (old != NULL) {
        old->hide();
        removeWidget(old);
    }

    if (w != NULL){
        addChildWidget(w);
        mItemList[r] = new QWidgetItemV2(w);
        w->show();
    } else {
        mItemList[r] = 0;
    }

    invalidate();
}

int 
WorkspacePanelLayout::minimumTitleWidth() const
{

    WorkspacePanel *q = qobject_cast<WorkspacePanel*>(parentWidget());

    int titleHeight = this->titleHeight();

    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);
    int fw = q->style()->pixelMetric(QStyle::PM_DockWidgetFrameWidth, 0, q);

    return titleHeight + 2*fw + 3*mw;
}


int 
WorkspacePanelLayout::titleHeight() const
{
    WorkspacePanel *q = qobject_cast<WorkspacePanel*>(parentWidget());

    QFontMetrics titleFontMetrics = q->fontMetrics();

    int mw = q->style()->pixelMetric(QStyle::PM_DockWidgetTitleMargin, 0, q);

    return titleFontMetrics.lineSpacing() + 2 * mw;
}


void 
WorkspacePanelLayout::setGeometry(const QRect& geometry)
{
    WorkspacePanel *q = qobject_cast<WorkspacePanel*>(parentWidget());

    QStyleOptionDockWidgetV2 opt;
    q->initStyleOption(&opt);

    QLayoutItem* item = mItemList[Content];
    if (item != NULL)
        item->setGeometry(geometry);
}


//=============================================================================
// class WorkspacePanel
//=============================================================================
int WorkspacePanel::sPanelCount = 0;


//-----------------------------------------------------------------------------
// WorkspacePanel::WorkspacePanel()
//-----------------------------------------------------------------------------
WorkspacePanel::WorkspacePanel(const QString& inTitle,
							   QWidget* inParent)
    :   QFrame(inParent)
    ,	mActive(false)
    ,   mHover(false)
    ,   mFeatures(WorkspacePanel::DockWidgetClosable
                | WorkspacePanel::DockWidgetMovable
                | WorkspacePanel::DockWidgetFloatable)
    ,   mAllowedAreas(Qt::AllDockWidgetAreas)
    ,   mToggleViewAction(NULL)
    ,   mTimerId(-1)
{
    // QCursor is lately very finicky about the bitmaps you pass it.
    // They MUST be monochromatic. Convert them here just to be sure.
    QBitmap bm1(":/themes/normal/cursors/panel_move.bmp");
    QBitmap bm2(":/themes/normal/cursors/panel_move_mask.bmp");
    mDragCursor = QCursor(QBitmap::fromImage(bm1.toImage(), Qt::MonoOnly),
                          QBitmap::fromImage(bm2.toImage(), Qt::MonoOnly), 0,0);


    init(inTitle);
}


WorkspacePanel::~WorkspacePanel()
{
    // Stop the mousewatcher
    if (mTimerId != -1) {
        killTimer(mTimerId);
        mTimerId = -1;
    }
}


//-----------------------------------------------------------------------------
// WorkspacePanel::init()
//-----------------------------------------------------------------------------
void
WorkspacePanel::init(const QString& inTitle)
{
    setObjectName(inTitle);
         
    // Set up the layout    
    WorkspacePanelLayout* layout = new WorkspacePanelLayout(this);
    layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // Set up the actions
    mToggleViewAction = new QAction(this);
    mToggleViewAction->setCheckable(true);
    mFixedWindowTitle = qt_setWindowTitle_helperHelper(windowTitle(), this);
    mToggleViewAction->setText(mFixedWindowTitle);
    QObject::connect(mToggleViewAction, 
                     SIGNAL(triggered(bool)),
                     this, 
                     SLOT(toggleView(bool)));

	setWindowTitle(inTitle);
}


void
WorkspacePanel::setWidget(QWidget* widget)
{
    WorkspacePanelLayout* layout = qobject_cast<WorkspacePanelLayout*>(this->layout());
    layout->setWidgetForRole(WorkspacePanelLayout::Content, widget);

    if (widget && widget->testAttribute(Qt::WA_DeleteOnClose)) {
        // widget wants to delete on close
        setAttribute(Qt::WA_DeleteOnClose);
    }
}


QWidget*
WorkspacePanel::widget() const
{
    WorkspacePanelLayout *layout = qobject_cast<WorkspacePanelLayout*>(this->layout());
    return layout->widgetForRole(WorkspacePanelLayout::Content);
}


void 
WorkspacePanel::setFeatures(WorkspacePanel::DockWidgetFeatures features)
{
    features &= DockWidgetFeatureMask;
    if (mFeatures == features)
        return;

    mFeatures = features;
    mToggleViewAction->setEnabled((mFeatures & DockWidgetClosable) == DockWidgetClosable);
    
    Q_EMIT featuresChanged(mFeatures);
    update();
}


void 
WorkspacePanel::setFloating(bool floating)
{
    setWindowState(floating);
}


void 
WorkspacePanel::changeEvent(QEvent* event)
{
    switch (event->type()) {
    
        case QEvent::ModifiedChange:
        case QEvent::WindowTitleChange:
            mFixedWindowTitle = qt_setWindowTitle_helperHelper(windowTitle(), this);
            mToggleViewAction->setText(mFixedWindowTitle);
            break;

        default:
            break;
    }

    QWidget::changeEvent(event);
}


/*! \reimp */
bool
WorkspacePanel::event(QEvent *event)
{
    QWidget* win = parentWidget();
    WorkspaceLayout* layout = NULL;
    if (win != NULL)
        layout = qobject_cast<WorkspaceLayout*>(win->layout());

    switch (event->type()) {
    
    case QEvent::Hide:
        //if (layout != NULL)
        //    layout->keepSize(this);
            
        mToggleViewAction->setChecked(false);
        Q_EMIT visibilityChanged(false);
        break;

    case QEvent::Show:
        mToggleViewAction->setChecked(true);
        Q_EMIT visibilityChanged(geometry().right() >= 0 && geometry().bottom() >= 0);
        break;

    case QEvent::ZOrderChange: {
        bool onTop = false;
        if (win != NULL) {
            const QObjectList &siblings = win->children();
            onTop = siblings.count() > 0 && siblings.last() == (QObject*)this;
        }
        
        //if (!isFloating() && layout != NULL && onTop)
        //    layout->raise(this);
            
        break;
    }
             
    case QEvent::Resize:
        if (isFloating() && layout != NULL)
            mUndockedGeometry = geometry();
        break;

    case QEvent::Move:
    {       
		if (isFloating())
            titleBarDragEvent(static_cast<QMoveEvent*>(event));
    }
    break;        

    case QEvent::Enter:
    {
        setCursor(Qt::ArrowCursor);
    }
    break;

    default:
        break;
    }
    
    return QWidget::event(event);
}


/*! \reimp */
void
WorkspacePanel::paintEvent(QPaintEvent* paintEvent)
{
    Q_UNUSED(paintEvent);

    // Get the painter
    QPainter painter(this);

    // We won't be doing any fills
    painter.setBrush(Qt::NoBrush);

    // Get the current style options
    QStyleOptionFrame frameOption;
    frameOption.init(this);

    // Get the outline of the frame
    QRect outline = rect();
    
    // Draw the top line using the window color so we don't
    // interfere with the blend between the tab and the frame.    
    painter.setPen(frameOption.palette.window().color());
    painter.drawLine(outline.topLeft(), outline.topRight());

    // Set uo the active pen
    QPen activePen(WorkspaceArea::ActivePanelColor);
    activePen.setWidth(1);

	if (mActive)
        painter.setPen(activePen);	
	else
    	painter.setPen(frameOption.palette.window().color().darker());
	
   	// Draw the left side
    painter.drawLine(outline.topLeft(), outline.bottomLeft());

    // Draw the bottom
	if (!mActive)
    	painter.setPen(frameOption.palette.dark().color());	
    painter.drawLine(outline.bottomLeft(), outline.bottomRight());

    // Draw the right side
    painter.drawLine(outline.topRight(), outline.bottomRight());	
}


/*!
  Returns a checkable action that can be used to show or hide this
  dock widget.

  The action's text is set to the dock widget's window title.

  \sa QAction::text QWidget::windowTitle
 */
QAction* 
WorkspacePanel::toggleViewAction() const
{
    return mToggleViewAction;
}


const QString&
WorkspacePanel::fixedWindowTitle() const
{
    return mFixedWindowTitle;
}


void 
WorkspacePanel::initStyleOption(QStyleOptionDockWidget *option) const
{
    if (option == NULL)
        return;

    option->initFrom(this);
    option->title = mFixedWindowTitle;
    option->closable = hasFeature(WorkspacePanel::DockWidgetClosable);
    option->movable = hasFeature(WorkspacePanel::DockWidgetMovable);
    option->floatable = hasFeature(WorkspacePanel::DockWidgetFloatable);
}

/*
 *
 * slots
 *
 */
void
WorkspacePanel::onSignal()
{
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// WorkspacePanelItem                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
WorkspacePanelItem::WorkspacePanelItem(WorkspacePanel *dockWidget)
    : QWidgetItem(dockWidget)
{
}

QSize WorkspacePanelItem::minimumSize() const
{
    QSize widgetMin(0, 0);
    if (QLayoutItem *item = dockWidgetChildItem())
        widgetMin = item->minimumSize();
    return dockWidgetLayout()->sizeFromContent(widgetMin);
}

QSize WorkspacePanelItem::maximumSize() const
{
    if (QLayoutItem *item = dockWidgetChildItem()) {
        return dockWidgetLayout()->sizeFromContent(item->maximumSize());
    } else {
        return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    }
}


QSize WorkspacePanelItem::sizeHint() const
{
    if (QLayoutItem *item = dockWidgetChildItem()) {
         return dockWidgetLayout()->sizeFromContent(item->sizeHint());
    } else {
        return QWidgetItem::sizeHint();
    }
}


//-----------------------------------------------------------------------------
// WorkspacePanel::titleBarDragEvent
//-----------------------------------------------------------------------------
void
WorkspacePanel::timerEvent(QTimerEvent* inEvent)
{
    Q_UNUSED(inEvent);

    // Get the layout
    QWidget* theWidget = parentWidget();
    WorkspaceLayout* theLayout = NULL;
    if (theWidget != NULL)
        theLayout = qobject_cast<WorkspaceLayout*>(theWidget->layout());

    if (theLayout == NULL)
        return;

    // Get the mouse button state
    const bool ctrlDrag = getCurrentKeyboardModifiers() & Qt::ControlModifier;
#ifdef Q_WS_MAC
    if (!::GetCurrentEventButtonState() & kEventMouseButtonPrimary) {
#else    
    if (!isMouseButtonDown(Button1Mask)) {
#endif    
        // Stop the mouse watcher
        killTimer(mTimerId);
        mTimerId = -1;

        // Check and see if we need to redock into the workspace.
        if (mHover && isOverWorkspace()) {
            mHover = false;
            theLayout->endHover(this);
        }
    } else {           
        if (ctrlDrag) {
            // Show and update the workspace hover 
            // while the key is pressed.
            theLayout->hover(this, QCursor::pos());
            mHover = true;
        } else {
            if (mHover) {
                // Hide the workspace hover indicator
                // when the control key is released   
                theLayout->endHover();
                mHover = false;
            }                
        }
    }         
}


//-----------------------------------------------------------------------------
// WorkspacePanel::startProgress
//
/// Activate the progress indicator.
//-----------------------------------------------------------------------------
void
WorkspacePanel::startProgress()
{
/*
    TODO gragan: Hook this up
	WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(parent());
	Q_ASSERT(theArea != NULL);
	
	WorkspaceLayout* theLayout =
		qobject_cast<WorkspaceLayout*>(theArea->layout());
	Q_ASSERT(theLayout != NULL);
*/
}


//-----------------------------------------------------------------------------
// WorkspacePanel::stopProgress
// 
/// Stop the progress indicator.
//-----------------------------------------------------------------------------
void
WorkspacePanel::stopProgress()
{
/*
    TODO gragan: Hook this up
	WorkspaceArea* theArea = qobject_cast<WorkspaceArea*>(parent());
	Q_ASSERT(theArea != NULL);
*/
}


//-----------------------------------------------------------------------------
// WorkspacePanel::toggleView
//-----------------------------------------------------------------------------
void 
WorkspacePanel::toggleView(bool b)
{
    if (b == isHidden()) {
        if (b)
            show();
        else
            hide();
    }
}


//-----------------------------------------------------------------------------
// WorkspacePanel::toggleTopLevel
//-----------------------------------------------------------------------------
void 
WorkspacePanel::toggleTopLevel()
{
    setFloating(!isFloating());
}



//-----------------------------------------------------------------------------
// WorkspacePanel::isOverWorkspace
// 
/// Check and see if the panel is floating over the workspace.
//-----------------------------------------------------------------------------
bool
WorkspacePanel::isOverWorkspace() const
{
    // Get the layout    
    QWidget* theWidget = parentWidget();
    WorkspaceLayout* theLayout = NULL;
    if (theWidget != NULL)
        theLayout = qobject_cast<WorkspaceLayout*>(theWidget->layout());

    if (theLayout == NULL)
        return false;
        
    // Get the global location of the workspace
    // TODO gragan: This only checks for the workspace. This will
    // have to be extended to handle docking into other undocked
    // panels when we get to the feature.
    const QRect wbBounds = theLayout->parentWidget()->geometry();
    const QPoint globalOrigin = theLayout->parentWidget()->mapToGlobal(wbBounds.topLeft());
    const QRect globalRect(globalOrigin.x(),
                           globalOrigin.y(),
                           wbBounds.width(),
                           wbBounds.height());

    return globalRect.contains(globalMousePos());
}


//-----------------------------------------------------------------------------
// WorkspacePanel::titleBarDragEvent
//-----------------------------------------------------------------------------
void
WorkspacePanel::titleBarDragEvent(QMoveEvent* inEvent)
{
    Q_UNUSED(inEvent);
  
    // Start observing the mouse button and keyboard state
    // during the titlebar drag. We can't use normal Qt 
    // event checking for this, as the window manager is
    // controlling the window.
    if (mTimerId == -1)
        mTimerId = startTimer(25);

    const bool ctrlDrag = getCurrentKeyboardModifiers() & Qt::ControlModifier;

    if (ctrlDrag && !isOverWorkspace()) {
        QWidget* theWidget = parentWidget();
        WorkspaceLayout* theLayout = NULL;
        if (theWidget != NULL) {
            theLayout = qobject_cast<WorkspaceLayout*>(theWidget->layout());
            if (theLayout != NULL) {
                mHover = false;
                theLayout->endHover();
            }
        }
    }
}


void 
WorkspacePanel::setWindowState(bool inFloating)
{
    const bool wasFloating = isFloating();
    const bool hidden = isHidden();

    Qt::WindowFlags flags = inFloating ? Qt::Window : Qt::Widget;

    // Calling setWindowFlags with the Qt::Window flag clear, when
    // previously it was set, causes SEVERE flashing on X11. The
    // entire desktop flashes. The only workaround I've found is
    // to use Qt::Tool instead of Qt::Window. This goes against
    // our design, which says that large panels NOT be always on
    // top of the main window.
    // This problem appears to be related to the window manager. It
    // occurs under metacity, but not kwin, for example.
    setWindowFlags(flags);

    if (!hidden) {
        QMetaObject::invokeMethod(this, "show", Qt::QueuedConnection);
        QMetaObject::invokeMethod(this, "raise", Qt::QueuedConnection);
    }

    if (inFloating != wasFloating) {
        Q_EMIT topLevelChanged(inFloating);
        if (!inFloating && parent()) {
            WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout *>(parentWidget()->layout());
            if (workspaceLayout != NULL) {
                workspaceLayout->addPanel(this, Qt::Horizontal);                

                // Since the panel will now be docked, remove it from the floating data structure
                //WorkspaceArea* workspaceArea = qobject_cast<WorkspaceArea*>(parent());
                //Q_ASSERT(workspaceArea != NULL);
                //workspaceArea->removeFromFloatingPanelPlaceHolders(objectName());

                Q_EMIT dockLocationChanged();
            }
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspacePanel::setActive
// 
/// Set the panel to the active state.
/// \param inActive The active state to set.
//-----------------------------------------------------------------------------
void
WorkspacePanel::setActive(bool inActive) 
{ 
    mActive = inActive; 
    update();
}


//-----------------------------------------------------------------------------
// WorkspacePanel::isFloating
// 
/// Check and see if the panel, or its panel group, is floating.
/// \result True if it, or its panel group, is floating.
//-----------------------------------------------------------------------------
bool
WorkspacePanel::isFloating() const 
{ 
    // This might be a single floating window
    if (isWindow())
        return true;

    // We are part of a tab group. Get the QTabWidget
    QWidget* tabWidget =  parentWidget()->parentWidget();
    Q_ASSERT(tabWidget != NULL);

    return tabWidget->isWindow();
}        

