#include <QBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>

#include "Dialog.h"
#include "stbsrisrates.h"

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
	auto quit = new QPushButton("Quit");
	connect->setToolTip("Connect to the host");
	host->setToolTip("Host a match of your own, or play by yourself");
	quit->setToolTip(":(");
	connect->setMaximumWidth(60);

	QObject::connect(host, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(connect, &QPushButton::clicked, [this]
	{
		if(address->text().length() == 0)
			QMessageBox::critical(this, "Error", "You cannot leave the address field blank");
		else
			accept();
	});
	QObject::connect(quit, &QPushButton::clicked, this, &QDialog::reject);

	hbox->addWidget(address);
	hbox->addWidget(connect);

	vbox->addLayout(hbox);
	vbox->addWidget(host);
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
