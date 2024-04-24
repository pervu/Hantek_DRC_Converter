QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../qcustomplot/qcustomplot.cpp \
    about.cpp \
    drcconverter.cpp \
    jsondrc.cpp \
    main.cpp \
    mainwindow.cpp \
    oscplot.cpp

HEADERS += \
    ../qcustomplot/qcustomplot.h \
    about.h \
    drcconverter.h \
    jsondrc.h \
    mainwindow.h \
    oscplot.h

FORMS += \
    about.ui \
    mainwindow.ui

INCLUDEPATH += $$PWD/qcustomplot
DEPENDPATH += $$PWD/qcustomplot

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES +=
