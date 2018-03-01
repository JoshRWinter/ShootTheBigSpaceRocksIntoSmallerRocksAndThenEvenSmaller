HEADERS += stbsrisrates.h
HEADERS += Dialog.h
HEADERS += Server.h
HEADERS += network.h
HEADERS += GameState.h
HEADERS += Lump.h
HEADERS += Log.h
HEADERS += Window.h
HEADERS += Asteroids.h

SOURCES += main.cpp
SOURCES += Dialog.cpp
SOURCES += Server.cpp
SOURCES += network.cpp
SOURCES += GameState.cpp
SOURCES += Log.cpp
SOURCES += Window.cpp
SOURCES += Asteroids.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets

TARGET = stbsrisrates
