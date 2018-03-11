#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QKeyEvent>
#include <QPixmap>

#include "Asteroids.h"

struct Assets
{
	enum class PackType
	{
		SIMPLE,
		FANCY
	};

	Assets(PackType t)
		: type(t)
	{
		// initialize asset pack variables
		switch(type)
		{
			case PackType::SIMPLE:
				path = "simple";
				pen = QPen(Qt::black);
				ship_health_pen = pen;
				bullet_pen = pen;
				health_brush = QBrush(Qt::black);
				break;
			case PackType::FANCY:
				path = "fancy";
				pen = QPen(Qt::white);
				ship_health_pen = QPen(QColor(45, 245, 45));
				bullet_pen = QPen(QColor(255, 255, 200));
				health_brush = QBrush(Qt::white);
				break;
			default:
				hcf("invalid pack type");
				break;
		}

		// construct players
		QPixmap initial_player(("assets/" + path + "/player.png").c_str());
		QPixmap initial_aster_big(("assets/" + path + "/asteroid_big.png").c_str());
		QPixmap initial_aster_med(("assets/" + path + "/asteroid_med.png").c_str());
		QPixmap initial_aster_small(("assets/" + path + "/asteroid_small.png").c_str());
		QPixmap initial_ship(("assets/" + path + "/cruiser.png").c_str());

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

	const PackType type;
	std::string path;
	QPen pen;
	QPen ship_health_pen;
	QPen bullet_pen;
	QBrush health_brush;
	QPixmap player[360], asteroid_big[360], asteroid_med[360], asteroid_small[360], ship[360];
};

class Window : public QWidget
{
public:
	Window(Assets::PackType, const std::string&, int);

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
