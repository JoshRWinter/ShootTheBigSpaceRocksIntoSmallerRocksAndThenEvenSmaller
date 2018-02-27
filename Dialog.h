#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace dlg
{

	class Greeter : public QDialog
	{
	public:
		Greeter();
		std::string addr() const;

	private:
		std::string connect_to;
	};

}

#endif // DIALOG_H
