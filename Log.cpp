#include <mutex>
#include <iostream>
#include <string>

#include "Log.h"

void log_stdout(const std::string &line)
{
	static std::mutex stdout_lock;
	std::lock_guard<std::mutex> lock(stdout_lock);
	std::cout << line << std::endl;
}
