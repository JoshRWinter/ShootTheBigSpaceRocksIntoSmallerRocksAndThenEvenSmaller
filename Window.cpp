#include <QPainter>
#include <QTimer>

#include "Window.h"

Window::Window(const std::string &addr, int secret)
	: state(NULL)
	, game(addr, secret)
{
	resize(WINDOW_WIDTH, WINDOW_HEIGHT);
	setWindowTitle("Shoot The Big Space Rocks Into Smaller Rocks And Then Even Smaller");

	auto timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &Window::step);
	timer->start(16);

	setMouseTracking(true);
}

void Window::step()
{
	game.input(controls);
	state = &(game.step());

	repaint();
}

void Window::paintEvent(QPaintEvent*)
{
	if(state == NULL)
		return;

	QPainter painter(this);

	// draw world boundaries
	{
		float x = WORLD_LEFT, y = WORLD_TOP;
		game.adjust_coords(x, y);
		painter.drawRect(x, y, WORLD_WIDTH, WORLD_HEIGHT);
	}

	// draw players
	for(const Player &player : state->player_list)
	{
		float x = player.x, y = player.y;
		game.adjust_coords(x, y);
		painter.drawEllipse(x, y, player.w, player.h);
	}

	// draw booletts
	for(const Bullet &bullet : state->bullet_list)
	{
		float x = bullet.x, y = bullet.y;
		game.adjust_coords(x, y);
		painter.drawEllipse(x, y, bullet.w, bullet.h);
	}
}

void Window::keyPressEvent(QKeyEvent *event)
{
	process_keys(event->key(), true);
}

void Window::keyReleaseEvent(QKeyEvent *event)
{
	process_keys(event->key(), false);
}

void Window::mousePressEvent(QMouseEvent*)
{
	controls.fire = true;
}

void Window::mouseReleaseEvent(QMouseEvent*)
{
	controls.fire = false;
}

void Window::mouseMoveEvent(QMouseEvent *event)
{
	const int x = event->x();
	const int y = event->y();

	const int player_x = WINDOW_WIDTH / 2;
	const int player_y = WINDOW_HEIGHT / 2;

	controls.angle = atan2f(y - player_y, x - player_x);
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
