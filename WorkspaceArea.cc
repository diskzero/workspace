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
#include "WorkspaceArea.h"

// Qt
#include <QApplication>
#include <QFile>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QStyleOptionTabV3>
#include <QtDebug>
#include <QTabWidget>
#include <QXmlStreamWriter>

// Local
#include "WorkspaceLayout.h"
#include "WorkspacePanel.h"
#include "WorkspacePanelGroup.h"
#include "WorkspaceTabBar.h"
#include "WorkspaceUtils.h"


//=============================================================================
// class WorkspaceArea::DragState
//=============================================================================
struct WorkspaceArea::DragState 
{

    QPoint pressedPos;
    bool pressed;
    bool moving;
    Qt::Orientations orientations;
    QList<QWidget*> panels; // A list of panels that are being dragged

    DragState()
        :   pressed(false)
        ,   moving(false)
        ,   orientations(0)
        {
        }

    void clear()
    {
        pressedPos = QPoint();
        panels.clear();
        pressed = false;
        moving = false;
        orientations = 0;
    }
};

QDebug operator << (QDebug dbg, const WorkspaceArea::DragState& dragState)
{
        dbg.nospace() << "moving:" << dragState.moving;
        dbg.nospace() << "orientations:" << dragState.orientations;
        return dbg;
}


//=============================================================================
// class WorkspaceArea
//=============================================================================
static const QString kPluginPrefix = "plugin:";
QColor WorkspaceArea::ActivePanelColor(135, 179, 251);
static const QString kDefaultLayoutName = "Unnamed Layout";


//-----------------------------------------------------------------------------
// WorkspaceArea::WorkspaceArea()
//-----------------------------------------------------------------------------
WorkspaceArea::WorkspaceArea(QWidget* parent)
    :   QWidget(parent)
    ,   mDragState(NULL)
    ,   mActiveGroup(NULL)
    ,   mActivePanel(NULL)
    ,   mLayoutName(kDefaultLayoutName)
{
    setObjectName("WorkspaceArea");

    init();
}


//-----------------------------------------------------------------------------
// WorkspaceArea::init()
//-----------------------------------------------------------------------------
void
WorkspaceArea::init()
{
    // We must set mouseTracking in order to receive the mouse events which
    // allow us to set the cursor when the mouse moves over a splitter.
    setMouseTracking(true);

    setAutoFillBackground(true);

//  setAttribute(Qt::WA_OpaquePaintEvent, true);

    mDragState = new DragState();

    // Create the layout and set up the signal redirection
    WorkspaceLayout* theLayout = new WorkspaceLayout(this);
    setLayout(theLayout);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::~WorkspaceArea()
//-----------------------------------------------------------------------------
WorkspaceArea::~WorkspaceArea()
{
    delete mDragState;
}


//-----------------------------------------------------------------------------
// WorkspaceArea::addPanel()
//
/// Add a panel to the Workspace.  If there is an active panel, this panel
/// will be added to the same group.
/// \param inPanel The panel to add.
/// \param inOrientation The orientation to add the panel.
//-----------------------------------------------------------------------------
void
WorkspaceArea::addPanel(WorkspacePanel* inPanel, Qt::Orientation inOrientation)
{
    // Get the layout
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(workspaceLayout != NULL);

    // Remove the panel if it is already there.
    workspaceLayout->removeWidget(inPanel);

    FloatingPanelPlaceHolder ph;
    if (findFloatingPanelPlaceHolder(inPanel->objectName(), ph)) {
        inPanel->show();
        inPanel->setFloating(true);
        inPanel->setGeometry(ph.geometry);
    } else {
        if (mActivePanel != NULL) {
            // Insert the new panel into the same group as the active panel.
            addToPanelGroup(mActiveGroup, inPanel);
        } else {
            // Add the panel to the default location.
            workspaceLayout->addPanel(inPanel, inOrientation);
        }            
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::addFloatingPanel()
//-----------------------------------------------------------------------------
void
WorkspaceArea::addFloatingPanel(WorkspacePanel* panel)
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout);
    workspaceLayout->removeWidget(panel); // In case its already there.

    if (panel->parent() != this)
        panel->setParent(this);

    panel->show();

    FloatingPanelPlaceHolder ph;
    if (findFloatingPanelPlaceHolder(panel->objectName(), ph)) {
        panel->setGeometry(ph.geometry);
    }

    panel->setFloating(true);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::insertPanel()
//-----------------------------------------------------------------------------
void
WorkspaceArea::insertPanel(WorkspacePanel* position,
                           WorkspacePanel* panel,
                           Qt::Orientation orientation,
                           bool before)
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout);
    workspaceLayout->removeWidget(panel); // In case its already there.
    workspaceLayout->insertPanel(position, panel, orientation, before);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::addToPanelGroup()
//-----------------------------------------------------------------------------
void
WorkspaceArea::addToPanelGroup(WorkspacePanelGroup* inGroup,
                               WorkspacePanel* panel)
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    workspaceLayout->removeWidget(panel); // In case its already there.
    workspaceLayout->addToPanelGroup(inGroup, panel);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::addToPanelGroup()
//-----------------------------------------------------------------------------
void
WorkspaceArea::addToPanelGroup(WorkspacePanel* sibling,
                               WorkspacePanel* panel)
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    workspaceLayout->removeWidget(panel); // In case its already there.
    workspaceLayout->addToPanelGroup(sibling, panel);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::removePanelGroup()
//
/// Remove the panel group.
/// \param inGroup The group to remove.
//-----------------------------------------------------------------------------
void 
WorkspaceArea::removePanelGroup(WorkspacePanelGroup* inGroup)
{
    Q_ASSERT(inGroup != NULL);

    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);

    workspaceLayout->removePanelGroup(inGroup); 
}


//-----------------------------------------------------------------------------
// WorkspaceArea::beginDeferLayout()
//-----------------------------------------------------------------------------
void
WorkspaceArea::beginDeferLayout()
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    workspaceLayout->beginDeferLayout();
}



//-----------------------------------------------------------------------------
// WorkspaceArea::endDeferLayout()
//-----------------------------------------------------------------------------
void
WorkspaceArea::endDeferLayout()
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    workspaceLayout->endDeferLayout();
}


//-----------------------------------------------------------------------------
// WorkspaceArea::removePanel()
//
/// Remove the panel from the workspace
/// \param inPanel The panel to be reomoved
//-----------------------------------------------------------------------------
void
WorkspaceArea::removePanel(WorkspacePanel* inPanel)
{
    Q_ASSERT(inPanel != NULL);

    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(theLayout != NULL);
    
    theLayout->removePanel(inPanel);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::getPanelList()
//
/// Return a list of all panels.
/// \param outList The list that will be populated with the panels.
//-----------------------------------------------------------------------------
void
WorkspaceArea::getPanelList(QList<WorkspacePanel*>& outList)
{
    // Get the layout
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(theLayout != NULL);

    // Prepare the list of panels
   outList.clear();

    // Get the panels from each of the panel groups
    WorkspaceLayout::GridConstIterator iter(theLayout->getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        Q_ASSERT(theGroup != NULL);

        // Add each panel to the list
        for (int index = 0; index < theGroup->count(); ++index) {
            WorkspacePanel* thePanel = qobject_cast<WorkspacePanel*>(theGroup->widget(index));
            if (thePanel != NULL)
                outList.push_back(thePanel);
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::getActivePanel()
//
/// Return the active panel.  It is possible for no active panels to
/// exist.
/// \result The active panel. NULL if there is no active panel.
//-----------------------------------------------------------------------------
WorkspacePanel* 
WorkspaceArea::getActivePanel()
{
    return mActivePanel;
}


//-----------------------------------------------------------------------------
// WorkspaceArea::setActivePanel()
//
/// Set the active panel.  There can only be one active panel per Workspace.
/// \param inPanel The panel to active.
//-----------------------------------------------------------------------------
void
WorkspaceArea::setActivePanel(WorkspacePanel* inPanel)
{
    // Don't reset the panel is already active.
    if (inPanel == mActivePanel)
        return;

    // Get the layout.
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(theLayout != NULL);

    // Make the last panel inactive	
    if (mActivePanel != NULL) {

        WorkspaceLayout::GridConstIterator iter(theLayout->getConstraintsMap());
        while (iter.hasNext()) {
            iter.next();       
            WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
            Q_ASSERT(theGroup != NULL);
            theGroup->setActive(false);
        }
        
        // Clear the active tab bar and panel            
    	mActivePanel->setActive(false);

        // Set the active panel to NULL.
        mActivePanel = NULL;
    }

    	
	// Set the new active panel
	if (inPanel != NULL) {
    	mActivePanel = inPanel;
    	mActivePanel->setActive(true);
    	mActivePanel->update();

        // Find the tab bar that belongs to this widget and activate it.
        WorkspaceLayout::GridConstIterator iter(theLayout->getConstraintsMap());
        while (iter.hasNext()) {
            iter.next();       
            WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
            Q_ASSERT(theGroup != NULL);

            // Look at all of the tabs and compare the widgets to see if they contain
            // the panel that was passed in.
            for (int index = 0; index < theGroup->count(); ++index) {
                // The tab data will contain a pointer to the widget.
                QWidget* itemWidget = theGroup->widget(index);
                WorkspacePanel* panel = qobject_cast<WorkspacePanel*>(itemWidget);
                if (panel == mActivePanel) {
                    theGroup->setActive(true);
                    break;
                }
            }
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::activateNextPanel()
//
/// Active the next panel in the group that contais the panel passed in.
/// If the panel passed in is the last panel in the group, the group
/// will wrap and activate the first panel in the group.
/// \param inPanel The panel to use.
//-----------------------------------------------------------------------------
void
WorkspaceArea::activateNextPanel(WorkspacePanel* inPanel)
{
    Q_ASSERT(inPanel != NULL);

    // Get the layout.
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(theLayout != NULL);

    // Get the tab bar that contains this panel
    WorkspaceLayout::GridConstIterator iter(theLayout->getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        Q_ASSERT(theGroup != NULL);

        // Look at all of the tabs and compare the widgets to see if they contain
        // the panel that was passed in.
        for (int index = 0; index < theGroup->count(); ++index) {
            // The tab data will contain a pointer to the widget.
            QWidget* itemWidget = theGroup->widget(index);
            WorkspacePanel* panel = qobject_cast<WorkspacePanel*>(itemWidget);
            if (panel == inPanel) {

                // We found the tab bar containing this panel.
                // Try to activate the next panel, wrapping around if needed.
                const int currentIndex = theGroup->currentIndex();
                if (currentIndex == theGroup->count() - 1) {
                    theGroup->setCurrentIndex(0);
                } else {
                    theGroup->setCurrentIndex(currentIndex + 1);
                }
                break;
            }
        }
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::saveState()
// 
/// Save the state of the panel groups.  This method can be used
/// to create a standalone file, or append the current group to 
/// another layout file.
/// \param inFile The file to save to.
/// \param inStandAlone If true, make layout self contained in file.
//----------------------------------------------------------------------------
static const char* kWorkspaceLayoutElement = "layout";
static const char* kGridElement = "grid";
static const char* kGroupsElement = "groups";
static const char* kGroupElement = "group";
static const char* kPanelElement = "panel";
static const char* kFloatersElement = "floaters";

static const char* kColumnsAttribute = "columns";
static const char* kRowsAttribute = "rows";
static const char* kCountAttribute = "count";
static const char* kXAttribute = "x";
static const char* kYAttribute = "y";
static const char* kWidthAttribute = "width";
static const char* kHeightAttribute = "height";
static const char* kGridXAttribute = "gridx";
static const char* kGridYAttribute = "gridy";
static const char* kGridWidthAttribute = "gridwidth";
static const char* kGridHeightAttribute = "gridheight";
static const char* kNameAttribute = "name";
static const char* kActivePanelAttribute = "active";

void
WorkspaceArea::saveState(QXmlStreamWriter& inStream, bool inStandAlone) const
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    
    if (inStandAlone)
        inStream.writeStartDocument();
    
    // Write out all of the info for the current workspace layout.
    inStream.writeStartElement(kWorkspaceLayoutElement);
    inStream.writeAttribute(kNameAttribute, mLayoutName);
    
    // Write out the grid size
    inStream.writeStartElement(kGridElement);
    inStream.writeAttribute(kColumnsAttribute, QString::number(workspaceLayout->columnCount()));
    inStream.writeAttribute(kRowsAttribute, QString::number(workspaceLayout->rowCount()));
    inStream.writeEndElement(); // kGridElement

    // Get the state
    SavedLayout savedLayout;
    saveState(savedLayout);
    SavedGroups& savedGroups = savedLayout.second;
    
    // Write out the panel groups
    inStream.writeStartElement(kGroupsElement);
    inStream.writeAttribute(kCountAttribute, QString::number(savedGroups.size()));

    QMapIterator<QString, SavedGroup> iter(savedGroups);
    while (iter.hasNext()) {
        iter.next();
        
        const SavedGroup& savedGroup = iter.value();

        inStream.writeStartElement(kGroupElement);
        inStream.writeAttribute(kNameAttribute, iter.key());
        inStream.writeAttribute(kGridXAttribute, QString::number(savedGroup.gridx));
        inStream.writeAttribute(kGridYAttribute, QString::number(savedGroup.gridy));
        inStream.writeAttribute(kGridWidthAttribute, QString::number(savedGroup.gridwidth));
        inStream.writeAttribute(kGridHeightAttribute, QString::number(savedGroup.gridheight));
        inStream.writeAttribute(kXAttribute, QString::number(savedGroup.x));
        inStream.writeAttribute(kYAttribute, QString::number(savedGroup.gridy));
        inStream.writeAttribute(kWidthAttribute, QString::number(savedGroup.width));
        inStream.writeAttribute(kHeightAttribute, QString::number(savedGroup.height));
        inStream.writeAttribute(kActivePanelAttribute, QString::number(savedGroup.active));

        Q_FOREACH(const QString& panelName, savedGroup.panels) {
            inStream.writeStartElement(kPanelElement);
            inStream.writeAttribute(kNameAttribute, panelName);
            inStream.writeEndElement(); // kPanelElement
        }                
        
        inStream.writeEndElement(); // kGroupElement
    }
      
    inStream.writeEndElement(); // kGroupsElement
    
    
    // Write out all of floating panel groups.
    inStream.writeStartElement(kFloatersElement);

    // Save state of floating windows
    QList<WorkspacePanel*> floatingPanels;
    QList<WorkspacePanel*> childPanels = findChildren<WorkspacePanel*>();
    Q_FOREACH(WorkspacePanel* thePanel, childPanels) {
        if (thePanel->isFloating())
            floatingPanels << thePanel;
    }

    inStream.writeAttribute(kCountAttribute, QString::number(floatingPanels.count()));
    Q_FOREACH(WorkspacePanel* thePanel, floatingPanels) {
        inStream.writeStartElement(kPanelElement);
        const QRect& frame = thePanel->frameGeometry();
        inStream.writeAttribute(kXAttribute, QString::number(frame.x()));
        inStream.writeAttribute(kYAttribute, QString::number(frame.y()));
        inStream.writeAttribute(kWidthAttribute, QString::number(frame.width()));
        inStream.writeAttribute(kHeightAttribute, QString::number(frame.height()));
        inStream.writeAttribute(kNameAttribute, thePanel->objectName());
        inStream.writeEndElement(); // kPanelElement
    }    
    
    inStream.writeEndElement(); // kFloatersElement

    inStream.writeEndElement();  // kWorkspaceLayoutElement

    if (inStandAlone)
        inStream.writeEndDocument();
}


//-----------------------------------------------------------------------------
// WorkspaceArea::saveState()
// 
/// Save the state of the panel groups into a SavedGroup structure.
/// \param outGroup The group to save the layout data into.
//----------------------------------------------------------------------------
void
WorkspaceArea::saveState(SavedLayout& outLayout) const
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
            
    // Write out the grid size
    //stream.writeStartElement(kGridElement);
    //stream.writeAttribute(kColumnsAttribute, QString::number(workspaceLayout->columnCount()));
    //stream.writeAttribute(kRowsAttribute, QString::number(workspaceLayout->rowCount()));
    //stream.writeEndElement(); // kGridElement

    QString groupName("Panel Group ");
    int groupIndex = 0;
    
    SavedGroups& savedGroups = outLayout.second;

    DynamicGridLayout::GridConstIterator iter(workspaceLayout->getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();
        
        QWidget* widget = iter.key();
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(widget);
        if (theGroup != NULL) {
            SavedGroup savedGroup;
            //savedGroup.name = theGroup->objectName();
            savedGroup.name = groupName + QString::number(groupIndex++);
            
            const QRect& frame = theGroup->frameGeometry();
            savedGroup.x = frame.x();
            savedGroup.y = frame.y();            
            savedGroup.width = frame.width();
            savedGroup.height = frame.height();

            const QPoint gridPos = workspaceLayout->itemLocation(widget);
            savedGroup.gridx = gridPos.x();
            savedGroup.gridy = gridPos.y();

            const QSize gridSize = workspaceLayout->itemSize(widget);
            savedGroup.gridwidth = gridSize.width();
            savedGroup.gridheight = gridSize.height();

            savedGroup.active = theGroup->currentIndex();

            // Get all of the panels in the group
            for (int panelIndex = 0; panelIndex < theGroup->count(); ++panelIndex) {            
                WorkspacePanel* panel = qobject_cast<WorkspacePanel*>(theGroup->widget(panelIndex));
                savedGroup.panels.push_back(panel->objectName());
            }

            // Save the group into the layout map            
            savedGroups[savedGroup.name] = savedGroup;    

        }
    }

    // Set the layout name           
    outLayout.first = mLayoutName;
}


//-----------------------------------------------------------------------------
// WorkspaceArea::saveState()
// 
/// Save the state of the panel groups into a SavedGroup structure.
/// \param inStream The stream to write into.
/// \param inGroups The groups to save into the file.
//----------------------------------------------------------------------------
void
WorkspaceArea::saveState(QXmlStreamWriter& inStream, const SavedGroups& inGroups) const
{
    // Write out the panel groups
    inStream.writeStartElement(kGroupsElement);
    inStream.writeAttribute(kCountAttribute, QString::number(inGroups.size()));

    QMapIterator<QString, SavedGroup> iter(inGroups);
    while (iter.hasNext()) {
        iter.next();
        
        const SavedGroup& savedGroup = iter.value();
                
        inStream.writeStartElement(kGroupElement);
        inStream.writeAttribute(kNameAttribute, iter.key());
        inStream.writeAttribute(kGridXAttribute, QString::number(savedGroup.gridx));
        inStream.writeAttribute(kGridYAttribute, QString::number(savedGroup.gridy));
        inStream.writeAttribute(kGridWidthAttribute, QString::number(savedGroup.gridwidth));
        inStream.writeAttribute(kGridHeightAttribute, QString::number(savedGroup.gridheight));
        inStream.writeAttribute(kXAttribute, QString::number(savedGroup.x));
        inStream.writeAttribute(kYAttribute, QString::number(savedGroup.gridy));
        inStream.writeAttribute(kWidthAttribute, QString::number(savedGroup.width));
        inStream.writeAttribute(kHeightAttribute, QString::number(savedGroup.height));
        inStream.writeAttribute(kActivePanelAttribute, QString::number(savedGroup.active));

        Q_FOREACH(const QString& panelName, savedGroup.panels) {
            inStream.writeStartElement(kPanelElement);
            inStream.writeAttribute(kNameAttribute, panelName);
            inStream.writeEndElement(); // kPanelElement
        }                
        
        inStream.writeEndElement(); // kGroupElement
    }
      
    inStream.writeEndElement(); // kGroupsElement  
}


//-----------------------------------------------------------------------------
// WorkspaceArea::restoreState()
// 
/// Restore the layout information from a file.
/// \param inFile The file to read.
/// \param outLayout The SavedGroups object to be populated.
/// \param inStandAlone If true, make layout self contained in file.
/// \result True if reading was successful.
//----------------------------------------------------------------------------
bool
WorkspaceArea::restoreState(QXmlStreamReader& inStream, SavedLayout& outLayout)
{
    bool foundStart = false;
    int groupIndex = 0;
    QString currentGroupName;
    SavedGroup layoutData;

    SavedGroups& theGroups = outLayout.second;

    while (!inStream.atEnd()) {
        switch (inStream.readNext()) {                
            case QXmlStreamReader::StartElement:
                {
                    if (inStream.name() == kWorkspaceLayoutElement) {
                        Q_ASSERT(!foundStart);

                        // Get the name of this layout
                        outLayout.first = inStream.attributes().value(kNameAttribute).toString();

                        foundStart = true;

                    } else if (inStream.name() == kGroupElement) {
                        Q_ASSERT(foundStart);

                        // Clear out the last group list and name
                        currentGroupName.clear();
                        layoutData.panels.clear();

                        // Get the group information
                        layoutData.name = inStream.attributes().value(kNameAttribute).toString();
                        layoutData.gridx = inStream.attributes().value(kGridXAttribute).toString().toInt();
                        layoutData.gridy = inStream.attributes().value(kGridYAttribute).toString().toInt();
                        layoutData.gridwidth = inStream.attributes().value(kGridWidthAttribute).toString().toInt();
                        layoutData.gridheight = inStream.attributes().value(kGridHeightAttribute).toString().toInt();
                        layoutData.x = inStream.attributes().value(kXAttribute).toString().toInt();
                        layoutData.y = inStream.attributes().value(kYAttribute).toString().toInt();
                        layoutData.width = inStream.attributes().value(kWidthAttribute).toString().toInt();
                        layoutData.height = inStream.attributes().value(kHeightAttribute).toString().toInt();
                        layoutData.active = inStream.attributes().value(kActivePanelAttribute).toString().toInt();
                    } else if (inStream.name() == kPanelElement) {
                        // Load the action info
                        QString panelName = inStream.attributes().value(kNameAttribute).toString();
                        layoutData.panels.push_back(panelName);
                    }
                }
                break;
                
            case QXmlStreamReader::EndElement:
                {
                    if (inStream.name() == kGroupElement) {
                        // Set the data for the group name
                        currentGroupName = "Panel Group " + QString::number(groupIndex++);
                        theGroups[currentGroupName] = layoutData;
                    }
                }
                break;

            case QXmlStreamReader::Invalid:
                qWarning() << inStream.errorString();
                break;

            default:
                break;
        }
    }

    return true;    
}


//-----------------------------------------------------------------------------
// WorkspaceArea::restoreComplete()
//
/// The workspace has been has been restored from settings
/// and all the panels have been created. Peform final
/// configuration, such as setting the current tab panel
/// for each tab group.
//-----------------------------------------------------------------------------
void
WorkspaceArea::restoreComplete()
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    
    // Make sure any empty spaces get filled
    workspaceLayout->fillEmptySpace();
    
    // Do the final layout
    workspaceLayout->updateLayout();    
}


//-----------------------------------------------------------------------------
// WorkspaceArea::mousePressEvent()
//----------------------------------------------------------------------------
void
WorkspaceArea::mousePressEvent(QMouseEvent* event)
{
    mDragState->clear();
    if (event->button() == Qt::LeftButton) {
        mDragState->pressedPos = event->pos();
        mDragState->pressed = true;
        
        // Update the drag cursor and populate the panels being resized.
        updateDragState(event->pos(), *mDragState);

        // Begin the layout panel resize
        WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
        Q_ASSERT(layout != NULL);        
        workspaceLayout->beginResizePanels(mDragState->orientations,
                                           mDragState->panels);        

        event->accept();
    } else {
        event->ignore();
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::mouseMoveEvent()
//----------------------------------------------------------------------------
void
WorkspaceArea::mouseMoveEvent(QMouseEvent* event)
{
    // Update the drag cursor
    updateDragState(event->pos(), *mDragState);
    
    if (mDragState->moving)
        updateSplitter(event->pos());

    adjustCursor();
    if (mDragState->orientations) {
        event->accept();
    } else {
        event->ignore();
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::mouseReleaseEvent()
//----------------------------------------------------------------------------
void
WorkspaceArea::mouseReleaseEvent(QMouseEvent* event)
{
    if (mDragState->moving) {
        updateSplitter(event->pos());
        mDragState->clear();
        unsetCursor();
        
        // End the layout panel resize
        WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
        Q_ASSERT(layout != NULL);        
        workspaceLayout->endResizePanels();
        
        event->accept();
    } else {
        mDragState->clear();
        event->ignore();
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::paintEvent()
//----------------------------------------------------------------------------
void
WorkspaceArea::paintEvent(QPaintEvent* paintEvent)
{
    QStyleOptionTabV3 option;
    option.initFrom(this);

    QPainter painter(this);
    const QVector<QRect>& exposedRects = paintEvent->region().rects();
    Q_FOREACH(const QRect& theRect, exposedRects) {
        painter.fillRect(theRect, option.palette.color(QPalette::Window).darker(130));
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::adjustCursor()
//----------------------------------------------------------------------------
void
WorkspaceArea::adjustCursor()
{
    switch (mDragState->orientations) {
    case 0: unsetCursor(); break;
    case Qt::Horizontal: setCursor(Qt::SplitHCursor ); break;
    case Qt::Vertical  : setCursor(Qt::SplitVCursor ); break;
    case 3             : setCursor(Qt::SizeAllCursor); break;
    }

}

namespace {

    /// Predicate that matches a FloatingPanelPlaceHolder by name
    ///
    struct NameMatchesPlaceHolder {
        bool operator()(const FloatingPanelPlaceHolder & it) const {
            return it.title == mTitle;
        };
        QString mTitle;
    };

} // anonymous namespace


//-----------------------------------------------------------------------------
// WorkspaceArea::findFloatingPanelPlaceHolder()
//----------------------------------------------------------------------------
bool
WorkspaceArea::findFloatingPanelPlaceHolder(const QString & name,
                                            FloatingPanelPlaceHolder& outResult) const 
{    
    NameMatchesPlaceHolder pred;
    pred.mTitle = name;

    QList<FloatingPanelPlaceHolder>::const_iterator needle =
        std::find_if(mFloatingPanelPlaceHolders.begin(),
                     mFloatingPanelPlaceHolders.end(),
                     pred);
    if (needle != mFloatingPanelPlaceHolders.end()) {
        outResult = *needle;
        return true;
    }
    return false;
}


//-----------------------------------------------------------------------------
// WorkspaceArea::updateDragState()
//
/// Find out if the given point is above a horizontal splitter, a vertical
/// splitter, or the intersection (T-shaped or cross-shaped) of a vertical
/// AND a horizontal splitter.
//----------------------------------------------------------------------------
void
WorkspaceArea::updateDragState(const QPoint& inPoint, DragState& inOutState) const
{
    if (inOutState.moving)
        return;

    // Initialize state to no splitter selected
    inOutState.panels.clear();

    if (inOutState.pressed) {
        if ((inPoint - inOutState.pressedPos).manhattanLength() > QApplication::startDragDistance()) {
            inOutState.moving = true;
            updateDragState2(inOutState.pressedPos, inOutState);
            return;
        }
    }
    updateDragState2(inPoint, inOutState);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::updateDragState2()
//----------------------------------------------------------------------------
void
WorkspaceArea::updateDragState2(const QPoint& inPoint, DragState& inOutState) const
{
    QWidget* w[4] = {0};
    w[0] = getPanelNearPoint(inPoint, -1, -1); // North West
    w[1] = getPanelNearPoint(inPoint,  1, -1); // North East
    w[2] = getPanelNearPoint(inPoint,  1,  1); // South East
    w[3] = getPanelNearPoint(inPoint, -1,  1); // South West

    //   w[0]              w[1]
    //
    // 
    //          inPoint
    //
    //    
    //   w[3]              w[2]
    Qt::Orientations &orientations = inOutState.orientations;

    // Reset orientations before setting new ones
    orientations = 0;

    for (int i = 0; i < 4; ++i) {
        QWidget* wCur = w[i];
        const int iNext = ((i+1) % 4);
        QWidget* wNext = w[iNext];
        if (wCur) {
             if (wNext && wCur != wNext) {
                 orientations |= (i%2) ? Qt::Vertical : Qt::Horizontal;
             }
             if (!inOutState.panels.contains(wCur)) {
                 inOutState.panels.push_back(wCur);
             }
        }
    }

    // Check for corner diffs, which would escape the logic of the previous
    // loop.
    if ((w[0]!=NULL) && (w[1]==NULL) && (w[2]!=NULL) && (w[3]==NULL) && (w[0] != w[2])) {
        orientations |= Qt::Horizontal;
        orientations |= Qt::Vertical;
    }
    if ((w[0]==NULL) && (w[1]!=NULL) && (w[2]==NULL) && (w[3]!=NULL) && (w[1] != w[3])) {
        orientations |= Qt::Horizontal;
        orientations |= Qt::Vertical;
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::updateSplitter()
//
/// A splitter has been moved by the user. Update the relevant splitter(s) to
/// the new position, and update any parts of the screen that need it.
//----------------------------------------------------------------------------
void
WorkspaceArea::updateSplitter(const QPoint& inPosition)
{
    WorkspaceLayout* workspaceLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(layout != NULL);
    if (mDragState->orientations)
        workspaceLayout->resizePanels(mDragState->orientations, inPosition);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::getPanelNearPoint()
//-----------------------------------------------------------------------------
QWidget*
WorkspaceArea::getPanelNearPoint(QPoint pos, int horzDx, int vertDx) const
{
    const int MAX_DISTANCE = 6;

    for (int i = 0; i < MAX_DISTANCE; ++i) {
        pos.rx() += horzDx;
        pos.ry() += vertDx;
        QWidget* wAtPos = childAt(pos);
        if (wAtPos != NULL) {

            WorkspacePanel* panelAtPos = WorkspaceUtils::findParent<WorkspacePanel>(wAtPos);
            if (panelAtPos != NULL)
                return panelAtPos;

            QTabWidget* tabBarAtPos = WorkspaceUtils::findParent<QTabWidget>(wAtPos);
            if (tabBarAtPos != NULL)
                return tabBarAtPos;
        }
    }
    return NULL;
}


//-----------------------------------------------------------------------------
// WorkspaceArea::removeFromFloatingPanelPlaceHolders()
//-----------------------------------------------------------------------------
void
WorkspaceArea::removeFromFloatingPanelPlaceHolders(const QString& name)
{
    NameMatchesPlaceHolder pred;
    pred.mTitle = name;
    QList<FloatingPanelPlaceHolder>::const_iterator needle =
        std::find_if(mFloatingPanelPlaceHolders.begin(),
                     mFloatingPanelPlaceHolders.end(),
                     pred);
                     
    if (needle != mFloatingPanelPlaceHolders.end())
        mFloatingPanelPlaceHolders.removeOne(*needle);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::closePanels()
// 
/// Close all panels.
//-----------------------------------------------------------------------------
void 
WorkspaceArea::closePanels()
{
    // Get the layout.
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(theLayout != NULL);

    // Get the panels from each of the panel groups
    WorkspaceLayout::GridConstIterator iter(theLayout->getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        Q_ASSERT(theGroup != NULL);
        removePanelGroup(theGroup);
    }
}


//-----------------------------------------------------------------------------
// WorkspaceArea::populateCreatePanelsMenu()
// 
/// Signaled when the panels menu needs to be updated.
/// \param inMenu The menu to populate.
//-----------------------------------------------------------------------------
void 
WorkspaceArea::populateCreatePanelsMenu(QMenu* inMenu)
{
    Q_EMIT updateCreatePanelsMenu(inMenu);
}


//-----------------------------------------------------------------------------
// WorkspaceArea::renamePanel()
//
/// Attempt to rename the active panel of the tab group.
/// \param inName The requested name.
/// \result True if the rename succeeded.
//-----------------------------------------------------------------------------
bool
WorkspaceArea::renamePanel(QTabWidget* inTabBar, const QString& inName)
{
    Q_ASSERT(inTabBar != NULL);

    // Clean up the name to remove whitespace
    QString renameString = inName.trimmed();

    // Get the active tab index
    const int activeIndex = inTabBar->currentIndex();
        
    // Do a check for an identical rename.
    if (inTabBar->tabText(activeIndex) == renameString)
        return true;

    // Get the layout.
    WorkspaceLayout* theLayout = qobject_cast<WorkspaceLayout*>(layout());
    Q_ASSERT(theLayout != NULL);

    // Check that we don't have a naming conflict.
    WorkspaceLayout::GridConstIterator iter(theLayout->getConstraintsMap());
    while (iter.hasNext()) {
        iter.next();       
        WorkspacePanelGroup* theGroup = qobject_cast<WorkspacePanelGroup*>(iter.key());
        Q_ASSERT(theGroup != NULL);

        for (int index = 0; index < theGroup->count(); ++index) {
            if (theGroup->tabText(index) == renameString)
                return false;
        }
    }
    
    // Rename the tab
    inTabBar->setTabText(activeIndex, renameString);
    inTabBar->setTabWhatsThis(activeIndex, renameString);

    // Update the object name
    QWidget* itemWidget = inTabBar->widget(activeIndex);
    WorkspacePanel* panel = qobject_cast<WorkspacePanel*>(itemWidget);
    panel->setWindowTitle(renameString);
   
    return true; 
}


//-----------------------------------------------------------------------------
// WorkspaceArea::getPanelAt()
//
/// Attempt to find a WorkspacePanel at the location.
/// \param inPoint The point to locate a panel at.
/// \result A pointer to the panel, or NULL if none is found.
//-----------------------------------------------------------------------------
WorkspacePanel*
WorkspaceArea::getPanelAt(const QPoint& inPoint) const
{
    WorkspacePanel* foundPanel = NULL;
    
    const QWidget* foundWidget = getPanelNearPoint(inPoint, 0, 0);
    if (foundWidget != NULL) {
        // We have found a widget. See if it is a WorkspacePanel.
        // It could be a WorkspacePanel, a QTabWidget or some other widget.
        QWidget* castWidget = const_cast<QWidget*>(foundWidget);
        foundPanel = qobject_cast<WorkspacePanel*>(castWidget);
    }

    return foundPanel;
}

