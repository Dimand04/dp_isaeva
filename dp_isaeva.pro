QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    authwidget.cpp \
    baseeditoritem.cpp \
    clickablelabel.cpp \
    clienteditordialog.cpp \
    dimensionitem.cpp \
    dooritem.cpp \
    editorscene.cpp \
    editorview.cpp \
    editorwindow.cpp \
    filestoragemanager.cpp \
    flooritem.cpp \
    foundationblockitem.cpp \
    main.cpp \
    mainwidget.cpp \
    materialeditordialog.cpp \
    nodeitem.cpp \
    objectitem.cpp \
    paymentdialog.cpp \
    projecteditordialog.cpp \
    projectestimatedialog.cpp \
    projectstagedialog.cpp \
    roleeditordialog.cpp \
    roofitem.cpp \
    simpledicteditordialog.cpp \
    textitem.cpp \
    usereditordialog.cpp \
    wallitem.cpp \
    windowitem.cpp

HEADERS += \
    authwidget.h \
    baseeditoritem.h \
    clickablelabel.h \
    clienteditordialog.h \
    dimensionitem.h \
    dooritem.h \
    editorscene.h \
    editorview.h \
    editorwindow.h \
    filestoragemanager.h \
    flooritem.h \
    foundationblockitem.h \
    mainwidget.h \
    materialeditordialog.h \
    nodeitem.h \
    objectitem.h \
    paymentdialog.h \
    projecteditordialog.h \
    projectestimatedialog.h \
    projectstagedialog.h \
    roleeditordialog.h \
    roofitem.h \
    simpledicteditordialog.h \
    textitem.h \
    usereditordialog.h \
    wallitem.h \
    windowitem.h

FORMS += \
    authwidget.ui \
    clienteditordialog.ui \
    editorwindow.ui \
    mainwidget.ui \
    materialeditordialog.ui \
    paymentdialog.ui \
    projecteditordialog.ui \
    projectestimatedialog.ui \
    projectstagedialog.ui \
    roleeditordialog.ui \
    simpledicteditordialog.ui \
    usereditordialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
