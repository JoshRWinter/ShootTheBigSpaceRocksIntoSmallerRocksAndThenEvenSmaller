#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QKeyEvent>

#include "Asteroids.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 500

class Window : public QWidget
{
public:
	Window(const std::string&, int);

private:
	void step();
	void paintEvent(QPaintEvent*);
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);
	void process_keys(int, bool);

	Controls controls;
	Asteroids game;
};

#endif // WINDOW_H
