
#define TRACE_LOG log_info("Entering line %d, file %s", __LINE__, __FILE__);

void log_error(const char *format, ...);
void log_info(const char *format, ...);
