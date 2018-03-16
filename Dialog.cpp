#include <QBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>

#include "Dialog.h"
#include "stbsrisrates.h"

static const char *const helptext =
"Welcome to Shoot The Big Space Rocks Into Smaller Rocks And Then Even Smaller (STBSRISRATES)!\n\n"
"Play by yourself or with a friend in a top-down, asteroid shooting, passenger cruiser defending arcade game.\n\n"
"\t[ Controls ]\n"
"- Use the WASD or Arrow Keys to navigate through the game world, and (left) click your mouse to fire the guns.\n\n"
"\t[ Asteroids ]\n"
"- Destroying one of the big asteroids will spawn 3 smaller asteroids, which will spawn smaller asteroids, and "
"then even smaller asteroids.\n"
"- The big asteroids yield 3 points, the medium asteroids 2 points, and the smallest 1 point.\n\n"
"\t[ Passenger Cruisers ]\n"
"- Defend the periodic passenger cruisers as they pass through the asteroid field.\n"
"- Assisting the cruiser in its escape without it being destroyed will yield 50 points.\n"
"- Failing to defend the cruiser will result in its destruction and a deduction of 50 points (even into the "
"negatives!).\n\n"
"\t[ Health & Friendly Fighters ]\n"
"- If you remain still (no movement other than turning around) without firing your guns, your health "
"will slowly regenerate on its own.\n"
"- If a friendly fighter becomes disabled, you can repair it by hovering your own vessel over the disabled "
"fighter. You will not be able to fire your guns while repairing a friendly fighter.\n\n"
"\t[ Game Over ]\n"
"- If all friendly fighters become disabled (including your own fighter), then the game is over and your final score will be displayed "
"center screen.\n"
"- Alternatively, If you reach the max score of 500, AND no cruise ship entities are active, "
"then the game is won, and you and your comrades can enjoy your victory as fireworks go off in the background.\n"
"- If you die in this game, you die in real life."
;

dlg::Greeter::Greeter()
{
	setWindowTitle("Welcome to STBSRISRATES!");
	setMinimumWidth(300);

	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	setLayout(vbox);

	address = new QLineEdit;
	address->setToolTip("Type in the IP Address of another player (who is hosting a match)");

	auto connect = new QPushButton("Go");
	auto host = new QPushButton("Host a Match");
	auto howto = new QPushButton("How to Play");
	auto quit = new QPushButton("Quit");
	connect->setToolTip("Connect to the host");
	host->setToolTip("Host a match of your own, or play by yourself");
	quit->setToolTip(":(");
	connect->setMaximumWidth(60);

	QObject::connect(host, &QPushButton::clicked, [this]
	{
		address->clear();
		accept();
	});
	QObject::connect(connect, &QPushButton::clicked, [this]
	{
		if(address->text().length() == 0)
			QMessageBox::critical(this, "Error", "You cannot leave the address field blank");
		else
			accept();
	});
	QObject::connect(quit, &QPushButton::clicked, this, &QDialog::reject);

	QObject::connect(howto, &QPushButton::clicked, [this]{ HelpBox(this); });

	hbox->addWidget(address);
	hbox->addWidget(connect);

	vbox->addLayout(hbox);
	vbox->addWidget(host);
	vbox->addWidget(howto);
	vbox->addWidget(quit);
}

std::string dlg::Greeter::addr() const
{
	return address->text().toStdString();
}

dlg::Connect::Connect(const std::string &address)
	: connector(address, SERVER_PORT)
	, addr(address)
	, start_time(time(NULL))
{
	setWindowTitle("Connecting...");

	auto vbox = new QVBoxLayout;
	setLayout(vbox);

	auto label = new QLabel(("Connecting to " + addr).c_str());
	auto cancel = new QPushButton("Cancel");

	vbox->addWidget(label);
	vbox->addWidget(cancel);

	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	if(!connector)
	{
		QTimer::singleShot(0, [this]
		{
			QMessageBox::critical(this, "Could not connect", ("Error: " + addr + " is not a valid address").c_str());
			reject();
		});

		return;
	}

	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, [this]
	{
		if(time(NULL) - start_time > 10)
		{
			QMessageBox::critical(this, "Could not connect", ("Could not connect to " + addr + ", timeout!").c_str());
			reject();
			return;
		}

		if(!connector.connect())
			return;

		timer->stop();

		std::uint8_t accepted = 0;
		connector.recv_block(&accepted, sizeof(accepted));
		if(!accepted)
		{
			QMessageBox::critical(this, "Could not connect", ("Could not connect to " + addr + ", server is full!").c_str());
			reject();
			return;
		}

		connector.recv_block(&udp_secret, sizeof(udp_secret));
		accept();
	});
	timer->start(300);
}

int dlg::Connect::secret() const
{
	return udp_secret;
}

void dlg::HelpBox(QWidget *parent)
{
	QMessageBox::information(parent, "STBSRISRATES", helptext);
}
