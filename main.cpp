#include <exception>
#include <memory>
#include <fstream>

#include <QApplication>
#include <QMessageBox>

#include "Dialog.h"
#include "Server.h"
#include "Window.h"

static int run(QApplication&);
static Assets::PackType get_asset_pack();

#ifdef _WIN32
static int argc = 0;
static char **const argv = NULL;
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

	Window window(get_asset_pack(), addr.length() > 0 ? addr : "127.0.0.1", connect.secret());
	window.show();

	return app.exec();
}

Assets::PackType get_asset_pack()
{
	const char *const path = "./assets/texture/active.txt";

	std::ifstream in(path);
	if(!in)
	{
		QMessageBox::warning(NULL, "Asset Pack", "Could not determine the active asset pack!");
		return Assets::PackType::FANCY;
	}

	std::string active;
	std::getline(in, active);

	if(active == "fancy")
		return Assets::PackType::FANCY;
	else if(active == "simple")
		return Assets::PackType::SIMPLE;
	else
		QMessageBox::warning(NULL, "Unrecognized Asset Pack Name", std::string("The Asset Pack \"" + active + "\" was not recognized.").c_str());

	return Assets::PackType::FANCY;
}
