#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QKeyEvent>
#include <QPixmap>

#include "Asteroids.h"

struct Assets
{
	Assets()
	{
		// construct players
		QPixmap initial_player("assets/player.png");
		QPixmap initial_aster_big("assets/asteroid_big.png");
		QPixmap initial_aster_med("assets/asteroid_med.png");
		QPixmap initial_aster_small("assets/asteroid_small.png");
		QPixmap initial_ship("assets/cruiser.png");

		for(int i = 0; i < 360; ++i)
		{
			QTransform transform = QTransform().rotate(i);

			player[i] = initial_player.transformed(transform, Qt::SmoothTransformation);
			asteroid_big[i] = initial_aster_big.transformed(transform, Qt::SmoothTransformation);
			asteroid_med[i] = initial_aster_med.transformed(transform, Qt::SmoothTransformation);
			asteroid_small[i] = initial_aster_small.transformed(transform, Qt::SmoothTransformation);
			ship[i] = initial_ship.transformed(transform, Qt::SmoothTransformation);
		}
	}

	static int todeg(float rad)
	{
		while(rad < 0.0f)
			rad += 3.1415926 * 2.0;
		return (int)(rad * (180.0 / 3.1415926)) % 360;
	}

	QPixmap &asteroid(AsteroidType type, float rad)
	{
		switch(type)
		{
			case AsteroidType::BIG:
				return asteroid_big[Assets::todeg(rad)];
			case AsteroidType::MED:
				return asteroid_med[Assets::todeg(rad)];
			case AsteroidType::SMALL:
				return asteroid_small[Assets::todeg(rad)];
			default: break;
		}

		hcf("invalid asteroid type");
	}

	QPixmap player[360], asteroid_big[360], asteroid_med[360], asteroid_small[360], ship[360];
};

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

	static int text_width(const QFontMetrics&, const QString&);

	QFont font_announcement;
	QFont font_fps;
	QFont font_health;
	QFont font_score;
	QFontMetrics fm_announcement;
	QFontMetrics fm_fps;
	QFontMetrics fm_health;
	QFontMetrics fm_score;

	Assets assets;
	Asteroids game;
	Controls controls;
};

#endif // WINDOW_H
