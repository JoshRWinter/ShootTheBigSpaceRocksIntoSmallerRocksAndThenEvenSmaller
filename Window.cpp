#include <QTimer>

#include "Window.h"

Window::Window(const std::string &addr, int secret)
	: game(addr, secret)
{
	resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	setWindowTitle("Shoot The Big Space Rocks Into Smaller Rocks And Then Even Smaller");

	auto timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &Window::step);
	timer->start(16);
}

void Window::step()
{
	game.input(controls);
	game.step();
	repaint();
}

void Window::paintEvent(QPaintEvent*)
{
}

void Window::keyPressEvent(QKeyEvent *event)
{
	process_keys(event->key(), true);
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
	process_keys(event->key(), false);
}

void Window::process_keys(int key, bool press)
{
	switch(key)
	{
		case Qt::Key_Up:
			controls.up = press;
			break;
		case Qt::Key_Down:
			controls.down = press;
			break;
		case Qt::Key_Right:
			controls.right = press;
			break;
		case Qt::Key_Left:
			controls.left = press;
			break;
	}
}
