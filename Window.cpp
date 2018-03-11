#include <QPainter>
#include <QTimer>
#include <QTransform>

#include "Window.h"

Window::Window(Assets::PackType pack, const std::string &addr, int secret)
	: font_announcement("sans-serif", 20)
	, font_fps("sans-serif", 10)
	, font_health("sans-serif", 12)
	, font_score("sans-serif", 15)
	, fm_announcement(font_announcement)
	, fm_fps(font_fps)
	, fm_health(font_health)
	, fm_score(font_score)
	, assets(pack)
	, game(addr, secret)
{
	resize(1000, 800);
	setWindowTitle("Shoot The Big Space Rocks Into Smaller Rocks And Then Even Smaller");

	// background color
	if(assets.type == Assets::PackType::FANCY)
	{
		QPalette palette;
		palette.setColor(QPalette::Background, QColor(10, 10, 10));
		setPalette(palette);
	}

	auto timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &Window::step);
	timer->start(16);

	setMouseTracking(true);
}

void Window::step()
{
	game.input(controls);
	game.step();

	repaint();
}

void Window::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	// draw world boundaries
	painter.setPen(assets.pen);
	{
		float x = WORLD_LEFT, y = WORLD_TOP;
		game.adjust_coords(this, x, y);
		painter.drawRect(x, y, WORLD_WIDTH, WORLD_HEIGHT);
	}

	// draw ships
	painter.setFont(font_health);
	painter.setPen(assets.ship_health_pen);
	QFontMetrics health_fm(font_health);
	for(const Ship &ship : game.state.ship_list)
	{
		float x = ship.x, y = ship.y;
		game.adjust_coords(this, x, y);
		const QPoint ship_center(x + (SHIP_WIDTH / 2), y + (SHIP_HEIGHT / 2));
		const QPixmap &rotated = assets.ship[Assets::todeg(ship.xv > 0.0f ? 0 : 3.1415926)];
		painter.drawPixmap(ship_center.x() - (rotated.width() / 2), ship_center.y() - (rotated.height() / 2), rotated.width(), rotated.height(), rotated);

		// draw health
		char health_str[10];
		snprintf(health_str, sizeof(health_str), "%d", ship.health < 0 ? 0 : ship.health);
		const int f_width = health_fm.width(health_str);
		const int f_height = health_fm.height();
		painter.drawText(x + (SHIP_WIDTH / 2) - (f_width / 2) + 1, y + (SHIP_HEIGHT / 2) - (f_height / 2), f_width, f_height, 0, health_str);
	}

	// draw asteroids
	for(const Asteroid &aster : game.state.asteroid_list)
	{
		float x = aster.x, y = aster.y;
		game.adjust_coords(this, x, y);
		const QPoint aster_center(x + (aster.w / 2), y + (aster.h / 2));
		const QPixmap &rotated = assets.asteroid(aster.type, aster.rot);
		painter.drawPixmap(aster_center.x() - (rotated.width() / 2), aster_center.y() - (rotated.height() / 2), rotated.width(), rotated.height(), rotated);
	}

	// draw players
	for(const Player &player : game.state.player_list)
	{
		float x = player.x, y = player.y;
		game.adjust_coords(this, x, y);
		// painter.drawEllipse(x, y, player.w, player.h);
		const QPoint player_center(x + (PLAYER_WIDTH / 2), y + (PLAYER_HEIGHT / 2));
		const QPixmap &rotated = assets.player[Assets::todeg(player.rot + 3.1415926)];
		painter.drawPixmap(player_center.x() - (rotated.width() / 2), player_center.y() - (rotated.height() / 2), rotated.width(), rotated.height(), rotated);
	}

	// draw booletts
	painter.setPen(assets.bullet_pen);
	for(const Bullet &bullet : game.state.bullet_list)
	{
		if(bullet.ttl > BULLET_TTL - 3)
			continue;

		float x = bullet.x + (bullet.w / 2), y = bullet.y + (bullet.h / 2);
		game.adjust_coords(this, x, y);
		const float mult = 2.0f;
		painter.drawLine(x, y, x - (bullet.xv * mult), y - (bullet.yv * mult));
	}

	// draw particles
	for(const Particle &particle : game.particle_list)
	{
		float x = particle.x, y = particle.y;
		const float len = 2.5f;
		float x2 = particle.x - (particle.xv * len), y2 = particle.y - (particle.yv * len);
		game.adjust_coords(this, x, y);
		game.adjust_coords(this, x2, y2);
		painter.drawLine(x, y, x2, y2);
	}

	// draw messages
	painter.setPen(assets.pen);
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

	// game over text
	bool alldead = true;
	for(const Player &p : game.state.player_list)
		if(p.health > 0)
		{
			alldead = false;
			break;
		}
	if(alldead && game.state.player_list.size() > 0)
	{
		const char *const go_str = "Game Over";
		char final_score_str[30];
		snprintf(final_score_str, sizeof(final_score_str), "Final Score: %d", game.score);
		painter.setFont(font_announcement);
		painter.drawText((width() / 2) - (fm_announcement.width(go_str) / 2), (height() / 2) - 100, go_str);
		painter.setFont(font_score);
		painter.drawText((width() / 2) - (fm_score.width(final_score_str) / 2), (height() / 2) - 100 + fm_announcement.height() + 10, final_score_str);
	}

	// win text
	if(game.win && !alldead)
	{
		const char *const victory_str = "Victory!";
		painter.setFont(font_announcement);
		painter.drawText((width() / 2) - (fm_announcement.width(victory_str) / 2), (height() / 2) - 100, victory_str);

		// draw fireworks
		for(const Firework &fw : game.firework_list)
		{
			float x = fw.x, y = fw.y;
			game.adjust_coords(this, x, y);
			painter.setBrush(QColor(fw.color.r, fw.color.g, fw.color.b));
			painter.drawEllipse(x, y, FIREWORK_SIZE, FIREWORK_SIZE);
		}

		painter.setBrush({});
	}

	// draw hud
	{
		const Player *const me = game.me();

		const int bar_width = 350;
		const int bar_height = 15;
		const int padding = 3;
		const int bar_x = (width() / 2) - (bar_width / 2);
		const int bar_y = height() - 40;
		const int health = me ? (me->health > 0 ? me->health : 0) : 100;

		painter.drawRect(bar_x, bar_y, bar_width, bar_height);
		painter.fillRect(QRect(bar_x + padding, bar_y + padding, (bar_width - (padding * 2)) * (health / 100.0) + (health > 0 ? 1 : 0), bar_height - (padding * 2) + 1), assets.health_brush);

		char score_str[20];
		snprintf(score_str, sizeof(score_str), "%d", game.score);
		painter.setFont(font_score);
		painter.drawText(bar_x + (bar_width / 2) - (fm_score.width(score_str) / 2), bar_y - 15, score_str);
	}

	// draw repair progress bar
	if(game.repair != 0)
	{
		const Player *const me = game.me();
		const int bar_width = 350;
		const int bar_height = 32;
		const int padding = 3;
		const int bar_x = (width() / 2) - (bar_width / 2);
		const int bar_y = (height() / 2) - 50;
		const int my_health = me ? me->health : 100;
		const int repair = game.repair;

		painter.drawRect(bar_x, bar_y, bar_width, bar_height);
		painter.fillRect(QRect(bar_x + padding, bar_y + padding, (bar_width - (padding * 2) + 1) * (repair / 100.0), bar_height - (padding * 2) + 1), QBrush(Qt::black));

		const char *const text = my_health > 0 ? "Repairing" : "Being Repaired";
		const int textwidth = fm_score.width(text);
		const int textheight = fm_score.height();
		painter.setPen(QPen(QColor(100, 100, 100)));
		painter.drawText(bar_x + (bar_width / 2) - (textwidth / 2), bar_y + (bar_height / 2) - (textheight / 2), textwidth, textheight, Qt::AlignCenter, text);
	}

	// handle the game being paused or not
	if(game.paused)
	{
		painter.setBrush(QBrush(QColor(0, 0, 0, 200)));
		painter.drawRect(0, 0, width(), height());
		painter.setPen(QPen(Qt::white));
		painter.setFont(font_announcement);
		const char *const pause_str = "PAUSED";
		painter.drawText((width() / 2) - (fm_announcement.width(pause_str) / 2), (height() / 2) - 100, pause_str);
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

	// check for fullscreen cmd F11
	if(event->key() == Qt::Key_F11)
		setWindowState(windowState() ^ Qt::WindowFullScreen);
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
		case Qt::Key_Escape:
			if(!press)
				controls.pause = !controls.pause;
			break;
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
