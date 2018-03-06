#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QKeyEvent>

#include "Asteroids.h"

class Window : public QWidget
{
public:
	Window(const std::string&, int);

private:
	void step();
	void paintEvent(QPaintEvent*);
	void keyPressEvent(QKeyEvent*);
	void keyReleaseEvent(QKeyEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void process_keys(int, bool);

	Asteroids game;
	Controls controls;
	const GameState *state;
	const std::vector<Particle> *const particle_list;
};

#endif // WINDOW_H
