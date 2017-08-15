QT += core
QT -= gui

CONFIG += c++11

TARGET = wst
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += \
    element.h \
    node.h \
    btree.h \
    workingsettree.h
