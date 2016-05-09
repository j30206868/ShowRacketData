#-------------------------------------------------
#
# Project created by QtCreator 2016-05-08T21:14:12
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ShowRacektPosture
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    math_3d.h

FORMS    += mainwindow.ui

LIBS += -lopengl32
LIBS += -lglu32
LIBS += "C:\Program Files (x86)\Windows Kits\8.0\Lib\win8\um\x86\glut32.lib"

