#ifndef LOG_H
#define LOG_H

void log_stdout(const char*, ...);
#define log(fmt, ...) log_stdout("%s:%s():%d: " fmt "\n", __FILE__, __func__, __LINE__, ##__VA_ARGS__)

#endif // LOG_H
