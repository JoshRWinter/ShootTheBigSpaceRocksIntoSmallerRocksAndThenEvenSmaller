#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDir>
#include <QKeyEvent>
#include <QPixmap>
#include <QGamepadManager>

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
				pause_screen_text_pen = QPen(Qt::white);
				health_brush = QBrush(Qt::black);
				pause_screen_brush = QBrush(QColor(50, 50, 50, 200));
				break;
			case PackType::FANCY:
				path = "fancy";
				pen = QPen(Qt::white);
				ship_health_pen = QPen(QColor(45, 245, 45));
				bullet_pen = QPen(QColor(255, 255, 200));
				pause_screen_text_pen = QPen(Qt::black);
				health_brush = QBrush(Qt::white);
				pause_screen_brush = QBrush(QColor(120, 120, 120, 100));
				break;
			default:
				hcf("invalid pack type");
				break;
		}

		// construct players
		QPixmap initial_player(("assets/texture/" + path + "/player.png").c_str());
		QPixmap initial_aster_big(("assets/texture/" + path + "/asteroid_big.png").c_str());
		QPixmap initial_aster_med(("assets/texture/" + path + "/asteroid_med.png").c_str());
		QPixmap initial_aster_small(("assets/texture/" + path + "/asteroid_small.png").c_str());
		QPixmap initial_ship(("assets/texture/" + path + "/cruiser.png").c_str());

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
	QPen pause_screen_text_pen;
	QBrush health_brush;
	QBrush pause_screen_brush;
	QPixmap player[360], asteroid_big[360], asteroid_med[360], asteroid_small[360], ship[360];
};

struct Sfx : QObject
{
	Sfx()
		: generator(time(NULL))
	{
		QObject::connect(&music_player, &QMediaPlayer::mediaStatusChanged, this, &Sfx::next);
		QDir sfx_dir("assets/sfx/music");

		const QFileInfoList &list = sfx_dir.entryInfoList();
		for(const QFileInfo &filename : list)
		{
			if(filename.fileName() == "." || filename.fileName() == "..")
				continue;

			musics.push_back(filename.absoluteFilePath());
		}

		index = musics.size();
	}

	void start()
	{
		next();
	}

	void forward()
	{
		music_player.stop();
		next();
	}

	void back()
	{
		const long long pos = music_player.position();

		if(pos > 1500)
			music_player.setPosition(0);
		else if(index > 1)
		{
			index -= 2;
			if(index < 0)
				index = 0;
			music_player.stop();
			next();
		}
	}

	void playpause()
	{
		const QMediaPlayer::State state = music_player.state();
		if(state == QMediaPlayer::State::PausedState)
			music_player.play();
		else if(state == QMediaPlayer::State::PlayingState)
			music_player.pause();
	}

private:
	void shuffle()
	{
		std::shuffle(musics.begin(), musics.end(), generator);
	}

	void next(QMediaPlayer::MediaStatus status = QMediaPlayer::MediaStatus::EndOfMedia)
	{
		if(status != QMediaPlayer::MediaStatus::EndOfMedia)
			return;
		if(musics.size() == 0)
			return;

		if((unsigned)index > musics.size() - 1)
		{
			// shuffle and reset
			shuffle();
			index = 0;
		}

		music_player.setMedia(QUrl::fromLocalFile(musics[index]));
		++index;
		music_player.play();
	}

	int index; // index into musics
	std::mt19937 generator; // mersenne twister
	std::vector<QString> musics;
	QMediaPlayer music_player;
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
	void set_gamepad_mode(bool);

	void gamepad_axis(int, QGamepadManager::GamepadAxis, double);
	void gamepad_button_press(int, QGamepadManager::GamepadButton, double);
	void gamepad_button_release(int, QGamepadManager::GamepadButton);
	void gamepad_button(QGamepadManager::GamepadButton, bool);
	void gamepad_button_pause(bool);

	static int text_width(const QFontMetrics&, const QString&);

	double axis_x, axis_y; // gamepad axis for right joystick
	bool gamepad_mode;

	QFont font_announcement;
	QFont font_fps;
	QFont font_health;
	QFont font_score;
	QFontMetrics fm_announcement;
	QFontMetrics fm_fps;
	QFontMetrics fm_health;
	QFontMetrics fm_score;

	Assets assets;
	Sfx sfx;
	Asteroids game;
	Controls controls;
};

#endif // WINDOW_H
