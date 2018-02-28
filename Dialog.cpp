#include <QBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>

#include "Dialog.h"

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
