#include <exception>
#include <memory>

#include <QApplication>
#include <QMessageBox>

#include "Dialog.h"
#include "Server.h"

static int run(QApplication&);

#ifdef _WIN32
static const int argc = 0;
static const char **const argv = NULL;
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT)
#else
int main(int argc, char **argv)
#endif // _WIN32
{
	srand(time(NULL));
	QApplication app(argc, argv);

	try
	{
		return run(app);
	}
	catch(const std::exception &e)
	{
		QMessageBox::critical(NULL, "Critical Error", e.what());
	}

	return 1;
}

int run(QApplication &app)
{
	std::unique_ptr<Server> server;

	// greet the user
	dlg::Greeter greeter;
	if(!greeter.exec())
		return 0;

	const std::string &addr = greeter.addr();
	if(addr.size() == 0) // hosting
		server.reset(new Server);

	// connect dialog
	dlg::Connect connect(addr.length() > 0 ? addr : "127.0.0.1");
	if(!connect.exec())
		return 1;

	QMessageBox::information(NULL, "secret", ("your secret is " + std::to_string(connect.secret())).c_str());

	return app.exec();
}
