TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    includes.h


unix|win32: LIBS += -lboost_system -lboost_thread -lssl -lcrypto -lpthread
