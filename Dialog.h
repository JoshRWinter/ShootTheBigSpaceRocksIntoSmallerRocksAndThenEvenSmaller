#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>

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

}

#endif // DIALOG_H
