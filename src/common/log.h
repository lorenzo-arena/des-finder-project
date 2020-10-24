#ifndef __LOG_H__
#define __LOG_H__

#define TRACE_LOG log_info("Entering line %d, file %s", __LINE__, __FILE__);

void log_error(const char *format, ...);
void log_info(const char *format, ...);

#endif
