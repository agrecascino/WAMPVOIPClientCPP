TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    timer.cpp \
    telnetdebugger.cpp \
    abstractor.cpp \
    reactor.cpp \
    includes.cpp \
    debugger.cpp \
    ncursesdisp.cpp \
    chatlogger.cpp \
    sanitymanager.cpp

HEADERS += \
    includes.h \
    timer.h \
    reactor.h \
    debugger.h \
    abstractor.h \
    structures.h \
    telnetdebugger.h \
    ncursesdisp.h \
    message.h \
    chatlogger.h \
    sanitymanager.h

DEFINES += LTM_DESC
QMAKE_CXXFLAGS += -std=c++14 -O0 -g

unix|win32: LIBS += -lboost_system -Wl,-Bdynamic  -lboost_thread -lboost_random -lpthread  -lssl -lpanel -lncurses -lalut -lopenal -lopus -lgcc -lcrypto
