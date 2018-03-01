#ifndef LOG_H
#define LOG_H

void log_stdout(const std::string&);
#define log(x) log_stdout(std::string(__FILE__) + " (" + __func__ + "():" + std::to_string(__LINE__) + "): " + x)

#endif // LOG_H
