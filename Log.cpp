#include <mutex>
#include <string>

#include <stdarg.h>
#include <stdio.h>

#include "Log.h"

void log_stdout(const char *prefix, const char *fmt, ...)
{
	static std::mutex stdout_lock;
	std::lock_guard<std::mutex> lock(stdout_lock);

	printf("%s", prefix);

	va_list list;
	va_start(list, fmt);
	vprintf(fmt, list);
	va_end(list);

	puts("");
	fflush(stdout);
}
