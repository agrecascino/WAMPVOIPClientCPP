TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    timer.cpp

HEADERS += \
    includes.h \
    timer.h

DEFINES += LTM_DESC
unix|win32: LIBS += -lboost_system -lboost_thread -lboost_random -lssl -lopus -lvorbisfile -lncurses -lalut -lopenal -lcrypto -lpthread -lgmp -ltomcrypt

QMAKE_CXXFLAGS += -std=c++14
