CONFIG += qt

QT += core gui widgets

INCLUDEPATH += ../

DEFINES += Q_WS_MAC

SOURCES += \
    mainwindow.cpp \
    main.cpp \
    ../DynamicGraphicsItems.cc \
    ../DynamicGridLayout.cc \    
    ../LayoutEngine.cc \
    ../WidgetAnimator.cc \
    ../Workspace.cc \
    ../WorkspaceArea.cc \
    ../WorkspaceItem.cc \
    ../WorkspaceLayout.cc \
    ../WorkspacePanelDropIndicator.cc \
    ../WorkspacePanel.cc \
    ../WorkspacePanelGroup.cc \
    ../WorkspaceTabBar.cc \
    ../theme/FrameworkStyle.cc       		\
	../theme/Theme.cc       				\

HEADERS += \
    mainwindow.h \
    ../DynamicGraphicsItems.h \
    ../DynamicGridLayout.h \    
    ../WidgetAnimator.h \
    ../Workspace.h \
    ../WorkspaceArea.h \
    ../WorkspaceItem.h \
    ../WorkspaceLayout.h \
    ../WorkspacePanel.h \
    ../WorkspacePanelGroup.h \
    ../WorkspacePanelDropIndicator.h \
    ../WorkspaceTabBar.h \
	../theme/FrameworkStyle.h       	\
	../theme/Theme.h       			\
    
RESOURCES =	workspace_demo.qrc			\

OBJECTS_DIR = ./obj

MOC_DIR = ./moc

RCC_DIR = ./res

macx {
CONFIG 		-= 	app_bundle
INCLUDEPATH	+= /System/Library/Frameworks/Carbon.framework/Headers
LIBS		= -framework Carbon
}
