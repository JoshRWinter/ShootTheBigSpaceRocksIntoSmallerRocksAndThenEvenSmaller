HEADERS += stbsrisrates.h
HEADERS += Dialog.h
HEADERS += Server.h
HEADERS += network.h
HEADERS += ServerState.h
HEADERS += Lump.h

SOURCES += main.cpp
SOURCES += Dialog.cpp
SOURCES += Server.cpp
SOURCES += network.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = stbsrisrates
