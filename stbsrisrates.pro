HEADERS += stbsrisrates.h
HEADERS += Dialog.h
HEADERS += Server.h
HEADERS += network.h
HEADERS += ServerState.h
HEADERS += Lump.h
HEADERS += Log.h

SOURCES += main.cpp
SOURCES += Dialog.cpp
SOURCES += Server.cpp
SOURCES += network.cpp
SOURCES += Log.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = stbsrisrates
