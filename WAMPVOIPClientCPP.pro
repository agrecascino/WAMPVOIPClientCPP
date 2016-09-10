TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    includes.h

DEFINES += LTM_DESC
unix|win32: LIBS += -lboost_system -lboost_thread -lssl -lcrypto -lpthread -ltfm -ltomcrypt -ltommath

QMAKE_CXXFLAGS += -std=c++14
