#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

#include "Dialog.h"

dlg::Greeter::Greeter()
{
	setWindowTitle("Welcome to STBSRISRATES!");

	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	setLayout(vbox);

	auto address = new QLineEdit;
	address->setToolTip("Type in the IP Address of another player (who is hosting a match)");

	auto connect = new QPushButton("Go");
	auto host = new QPushButton("Host a Match");
	auto quit = new QPushButton("Quit");
	connect->setToolTip("Connect to the host");
	host->setToolTip("Host a match of your own, or play by yourself");
	quit->setToolTip(":(");
	connect->setMaximumWidth(60);

	hbox->addWidget(address);
	hbox->addWidget(connect);

	vbox->addLayout(hbox);
	vbox->addWidget(host);
	vbox->addWidget(quit);
}

std::string dlg::Greeter::addr() const
{
	return connect_to;
}
