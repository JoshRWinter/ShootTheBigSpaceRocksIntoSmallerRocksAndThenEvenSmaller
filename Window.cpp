#include <QPainter>
#include <QTimer>
#include <QTransform>

#include "Window.h"

Window::Window(const std::string &addr, int secret)
	: game(addr, secret)
	, state(NULL)
	, particle_list(&(game.get_particles()))
{
	resize(1000, 800);
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

	// font related schtuff
	QFont font_announcement = painter.font();
	QFont font_fps = painter.font();
	QFont font_health(font_fps);
	font_announcement.setPointSize(20);
	font_fps.setPointSize(10);
	font_health.setPointSize(12);

	// draw world boundaries
	{
		float x = WORLD_LEFT, y = WORLD_TOP;
		game.adjust_coords(this, x, y);
		painter.drawRect(x, y, WORLD_WIDTH, WORLD_HEIGHT);
	}

	// draw ships
	painter.setFont(font_health);
	QFontMetrics health_fm(font_health);
	for(const Ship &ship : state->ship_list)
	{
		float x = ship.x, y = ship.y;
		game.adjust_coords(this, x, y);
		const QPoint ship_center(x + (SHIP_WIDTH / 2), y + (SHIP_HEIGHT / 2));
		const QPixmap &rotated = assets.ship[Assets::todeg(ship.xv > 0.0f ? 0 : 3.1415926)];
		painter.drawPixmap(ship_center.x() - (rotated.width() / 2), ship_center.y() - (rotated.height() / 2), rotated.width(), rotated.height(), rotated);

		// draw health
		char health_str[10];
		snprintf(health_str, sizeof(health_str), "%d%%", ship.health < 0 ? 0 : ship.health);
		const int f_width = health_fm.width(health_str);
		const int f_height = health_fm.height();
		painter.drawText(x + (SHIP_WIDTH / 2) - (f_width / 2), y + (SHIP_HEIGHT / 2) - (f_height / 2), f_width, f_height, 0, health_str);
	}

	// draw asteroids
	for(const Asteroid &aster : state->asteroid_list)
	{
		float x = aster.x, y = aster.y;
		game.adjust_coords(this, x, y);
		const QPoint aster_center(x + (aster.w / 2), y + (aster.h / 2));
		const QPixmap &rotated = assets.asteroid(aster.type, aster.rot);
		painter.drawPixmap(aster_center.x() - (rotated.width() / 2), aster_center.y() - (rotated.height() / 2), rotated.width(), rotated.height(), rotated);
	}

	// draw players
	for(const Player &player : state->player_list)
	{
		float x = player.x, y = player.y;
		game.adjust_coords(this, x, y);
		// painter.drawEllipse(x, y, player.w, player.h);
		const QPoint player_center(x + (PLAYER_WIDTH / 2), y + (PLAYER_HEIGHT / 2));
		const QPixmap &rotated = assets.player[Assets::todeg(player.rot + 3.1415926)];
		painter.drawPixmap(player_center.x() - (rotated.width() / 2), player_center.y() - (rotated.height() / 2), rotated.width(), rotated.height(), rotated);
	}

	// draw booletts
	for(const Bullet &bullet : state->bullet_list)
	{
		if(bullet.ttl > BULLET_TTL - 3)
			continue;

		float x = bullet.x + (bullet.w / 2), y = bullet.y + (bullet.h / 2);
		game.adjust_coords(this, x, y);
		const float mult = 2.0f;
		painter.drawLine(x, y, x - (bullet.xv * mult), y - (bullet.yv * mult));
	}

	// draw particles
	// painter.setPen({Qt::black, 2});
	for(const Particle &particle : *particle_list)
	{
		float x = particle.x, y = particle.y;
		const float len = 2.5f;
		float x2 = particle.x - (particle.xv * len), y2 = particle.y - (particle.yv * len);
		game.adjust_coords(this, x, y);
		game.adjust_coords(this, x2, y2);
		painter.drawLine(x, y, x2, y2);
	}

	// draw hud text
	if(!game.announcements.empty())
	{
		Announcement &msg = game.announcements.front();

		QFontMetrics fm(font_announcement);
		painter.setFont(font_announcement);
		const int w = text_width(fm, msg.say.c_str());
		const int x = (width() / 2) - (w / 2);
		painter.drawText(x, 100, w, 90, Qt::AlignCenter, msg.say.c_str());

		msg.timer -= game.delta;
		if(msg.timer <= 0.0f)
			game.announcements.pop();
	}

	// fps
	{
		static int fps, last;
		static char fpsstr[30] = "";
		const int now = time(NULL);

		if(now != last)
		{
			last = now;
			sprintf(fpsstr, "%d fps", fps > 60 ? 60 : fps);
			fps = 0;
		}
		else
			++fps;

		painter.setFont(font_fps);
		painter.drawText(QPointF(10.0, 10.0), fpsstr);
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

	const int player_x = width() / 2;
	const int player_y = height() / 2;

	controls.angle = atan2f(y - player_y, x - player_x);
}

void Window::process_keys(int key, bool press)
{
	switch(key)
	{
		// ARROW KEYS
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

		// QWERTY WASD
		case Qt::Key_W:
			controls.up = press;
			break;
		case Qt::Key_A:
			controls.left = press;
			break;
		case Qt::Key_S:
			controls.down = press;
			break;
		case Qt::Key_D:
			controls.right = press;
			break;

		// DVORAK WASD
		case Qt::Key_Comma:
			controls.up = press;
			break;
		case Qt::Key_O:
			controls.down = press;
			break;
		case Qt::Key_E:
			controls.right = press;
			break;
	}
}

int Window::text_width(const QFontMetrics &fm, const QString &text)
{
	int maxwidth = 0.0f;
	QStringList list = text.split("\n");
	for(const QString &entry : list)
	{
		const int width = fm.width(entry);
		if(width > maxwidth)
			maxwidth = width;
	}

	return maxwidth;
}
