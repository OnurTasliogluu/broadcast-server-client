#ifndef DEBUG_H
#define DEBUG_H

#define red     "\033[1;31m"
#define green   "\033[1;32m"
#define yellow  "\033[1;33m"
#define blue    "\033[1;34m"
#define magenta "\033[1;35m"
#define cyan    "\033[1;36m"
#define none    "\033[0m"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)  \
		fprintf(stdout, "%12s: %3d: %s:%s ", __FILE__, __LINE__, PS_NAME, green), \
		fprintf(stdout, "%s:%d:%s(): " fmt, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#endif