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
    editorscene.cpp \
    editorview.cpp \
    editorwindow.cpp \
    filestoragemanager.cpp \
    foundationblockitem.cpp \
    main.cpp \
    mainwidget.cpp \
    materialeditordialog.cpp \
    paymentdialog.cpp \
    projecteditordialog.cpp \
    projectestimatedialog.cpp \
    projectstagedialog.cpp \
    roleeditordialog.cpp \
    simpledicteditordialog.cpp \
    usereditordialog.cpp \
    wallitem.cpp \
    windowitem.cpp

HEADERS += \
    authwidget.h \
    baseeditoritem.h \
    clickablelabel.h \
    clienteditordialog.h \
    editorscene.h \
    editorview.h \
    editorwindow.h \
    filestoragemanager.h \
    foundationblockitem.h \
    mainwidget.h \
    materialeditordialog.h \
    paymentdialog.h \
    projecteditordialog.h \
    projectestimatedialog.h \
    projectstagedialog.h \
    roleeditordialog.h \
    simpledicteditordialog.h \
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
