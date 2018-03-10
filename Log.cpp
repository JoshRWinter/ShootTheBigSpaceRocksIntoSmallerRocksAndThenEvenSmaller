#include <mutex>
#include <string>

#include <stdarg.h>
#include <stdio.h>

#include "Log.h"

void log_stdout(const char *fmt, ...)
{
	static std::mutex stdout_lock;
	std::lock_guard<std::mutex> lock(stdout_lock);

	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);

	fflush(stdout);
}
