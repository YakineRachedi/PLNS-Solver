#pragma once

/******************************************************************************
 * Simple logging functions.
 *
 * The log message can include the time, the source file and the line number.
 *
 * Use LOG_MSG() instead of calling log_impl() directly. The macro adds
 * the file name and line number automatically.
 *
 * Initialization:
 *   log_init() sets the function used to print log messages.
 *   If NULL is passed, printf is used.
 *
 * Example:
 *   LOG_MSG("Number of vertices: %zu", count);
 ******************************************************************************/

#include <stdarg.h>

#ifdef __cplusplus
    extern "C" {
#endif

/* Set the function used to print log messages. */
void log_init(void (*logfunc)(const char *));

/* Clean up the logging system. */
void log_fini();

/* Internal logging functions. Prefer using LOG_MSG(). */
void log_impl(const char *filename, int line, const char *fmt, ...);
void vlog_impl(const char *filename, int line, const char *fmt, va_list args);


/* Macro helpers */

/*
 * Use the compiler built-in version when available.
 * It can be optimized by the compiler.
 */
#if defined(__GNUC__) || defined(__clang__)
#define BUILTIN_STRRCHR __builtin_strrchr
#else
#define BUILTIN_STRRCHR strrchr
#endif


/*
 * Get only the file name from a full path.
 *
 * Example:
 *   "/home/PLNS-Solver/src/mesh.cpp" -> "mesh.cpp"
 *   "C:\\PLNS-Solver\\src\\mesh.cpp" -> "mesh.cpp"
 */
#define LOG_BASENAME(f)                                                     \
	(BUILTIN_STRRCHR(f, '/') ?                                          \
		 BUILTIN_STRRCHR(f, '/') + 1 :                              \
		 (BUILTIN_STRRCHR(f, '\\') ? BUILTIN_STRRCHR(f, '\\') + 1 : \
					     (f)))


/*
 * Log a message with the source file and line number.
 *
 * Example:
 *   LOG_MSG("Loading mesh");
 *
 * This is expanded by the preprocessor to a call to log_impl()
 * with __FILE__ and __LINE__.
 */
#define LOG_MSG(...) log_impl(LOG_BASENAME(__FILE__), __LINE__, __VA_ARGS__)


/*
 * Define ASSERT_ALWAYS only if it already exists.
 * The existing definitions are replaced by the versions below.
 */
#ifdef ASSERT_ALWAYS

#undef ASSERT_ALWAYS
#undef ASSERT


/*
 * Check a condition and stop the program if it is false.
 *
 * The condition is always checked, even in release builds.
 */
#define ASSERT_ALWAYS(x)                                           \
	do {                                                       \
		if (UNLIKELY(!(x))) {                              \
			log_impl(LOG_BASENAME(__FILE__), __LINE__, \
				 "Assertion \"%s\" failed.", #x);  \
			abort();                                   \
		}                                                  \
	} while (0)


/*
 * ASSERT() is enabled in debug builds.
 *
 * In release builds, the expression is only evaluated and its
 * result is ignored.
 */
#ifndef NDEBUG
#define ASSERT(x) ASSERT_ALWAYS(x)
#else
#define ASSERT(x) (void)(x)
#endif

#endif


#ifdef __cplusplus
}
#endif

