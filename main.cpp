#include <exception>

#include <QApplication>
#include <QMessageBox>

#include "Dialog.h"

static void run();

#ifdef _WIN32
static const int argc = 0;
static const char **const argv = NULL;
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT)
#else
int main(int argc, char **argv)
#endif // _WIN32
{
	QApplication app(argc, argv);

	try
	{
		run();
	}
	catch(const std::exception &e)
	{
		QMessageBox::critical(NULL, "Critical Error", e.what());

		return 1;
	}

	return app.exec();
}

void run()
{
	// greet the user
	dlg::Greeter greeter;
	if(greeter.exec())
	{
		const std::string &addr = greeter.addr();
		QMessageBox::information(NULL, "info", (std::string("you are ") + (addr.length() > 0 ? "" : "not") + " hosting a match").c_str());
	}
}
