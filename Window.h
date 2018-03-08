#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QKeyEvent>
#include <QPixmap>

#include "Asteroids.h"

struct Assets
{
	Assets()
		: player("assets/player.png")
		, asteroid_big("assets/asteroid_big.png")
		, asteroid_med("assets/asteroid_med.png")
		, asteroid_small("assets/asteroid_small.png")
		, ship("assets/cruiser.png")
	{}

	QPixmap &asteroid(AsteroidType type)
	{
		switch(type)
		{
			case AsteroidType::BIG:
				return asteroid_big;
			case AsteroidType::MED:
				return asteroid_med;
			case AsteroidType::SMALL:
				return asteroid_small;
			default: break;
		}

		hcf("invalid asteroid type");
	}

	QPixmap player, asteroid_big, asteroid_med, asteroid_small, ship;
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

	Assets assets;
	Asteroids game;
	Controls controls;
	const GameState *state;
	const std::vector<Particle> *const particle_list;
};

#endif // WINDOW_H
