#ifndef LOG_H
#define LOG_H

void log_stdout(const char*, const char*, ...);
#define log(fmt, ...) log_stdout((std::string(__FILE__) + " (" + __func__ + "():" + std::to_string(__LINE__) + "): ").c_str(), fmt "\n", ##__VA_ARGS__)

#endif // LOG_H
