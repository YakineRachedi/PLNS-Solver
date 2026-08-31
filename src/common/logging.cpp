#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "logging.h"
#include "sys_utils.h"

#define DATE_FMT "%Y-%m-%d %H:%M:%S"
#define PREFIX_FMT "%s [%s:%d]: ", timedate, filename, line

/* Function used to output log messages. */
static void (*logfunc)(const char *) = NULL;


/* Default log function: print the message to stdout. */
void logprint(const char *buf) { printf("%s", buf); }


/*
 * Initialize the logging system.
 *
 * Use the user-provided function if it is not NULL.
 * Otherwise, use logprint().
 */
void log_init(void (*func)(const char *)) {logfunc = func ? func : logprint;}


/* Disable logging. */
void log_fini() {logfunc = NULL;}


/*
 * Log a formatted message.
 *
 * This function handles the variable arguments and passes them
 * to vlog_impl(), which builds and prints the final message.
 */
void log_impl(const char *filename, int line, const char *fmt, ...) {
	if (!logfunc)
		return;

	va_list args;
	va_start(args, fmt);

	vlog_impl(filename, line, fmt, args);

	va_end(args);
}


/*
 * Build and output a log message.
 *
 * The final message contains:
 *   - the current date and time
 *   - the source file name
 *   - the source line number
 *   - the user message
 */
void vlog_impl(const char *filename, int line, const char *fmt, va_list args) {
	if (!logfunc)
		return;

	/* Get the current date and time. */
	time_t t = time(NULL);
	struct tm *now = localtime(&t);

	char timedate[24];
	if (!strftime(timedate, sizeof(timedate), DATE_FMT, now)) {
		logfunc("Internal error. Cannot log timedate.\n");
		abort();
	}

	/*
	 * Calculate the size needed for the message prefix and the
	 * formatted message before allocating the buffer.
	 */
	size_t prefix_len = snprintf(NULL, 0, PREFIX_FMT);

	va_list args_copy;
	va_copy(args_copy, args);

	size_t len = vsnprintf(NULL, 0, fmt, args_copy);

	/*
	 * Allocate enough space for:
	 *   - the prefix
	 *   - the message
	 *   - the final '\n'
	 *   - the terminating '\0'
	 */
	char *buf = (char *)malloc(prefix_len + len + 2);
	if (!buf) {
		logfunc("Internal error. Out of memory while logging.\n");
		abort();
	}

	/* Write the prefix and the formatted message into the buffer. */
	(void)snprintf(buf, prefix_len + 1, PREFIX_FMT);
	(void)vsnprintf(&buf[prefix_len], len + 1, fmt, args);

	/* Add a newline at the end of the message. */
	(void)snprintf(&buf[strlen(buf)], 2, "\n");

	/* Send the complete message to the selected log function. */
	logfunc(buf);

	free(buf);
}