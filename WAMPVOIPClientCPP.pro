TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += \
    includes.h

DEFINES += LTM_DESC
unix|win32: LIBS += -lboost_system -lboost_thread -lssl -lopus -lportaudio -lcrypto -lpthread -ltfm -ltomcrypt -ltommath

QMAKE_CXXFLAGS += -std=c++14 -O0 -g3 -fstack-protector-all
