#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTimer>

#include "network.h"

namespace dlg
{
	class Greeter : public QDialog
	{
	public:
		Greeter();
		std::string addr() const;

	private:
		QLineEdit *address;
	};

	class Connect : public QDialog
	{
	public:
		Connect(const std::string&);
		int secret() const;

	private:
		std::int32_t udp_secret;
		QTimer *timer;
		net::tcp connector;
		const std::string addr;
		const int start_time;
	};

	void HelpBox(QWidget* = NULL);
}

#endif // DIALOG_H
