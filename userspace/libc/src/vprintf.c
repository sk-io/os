/*
 * vprintf.c
 */

#include <stdio.h>
#include <stdarg.h>

#include <os.h>

int vprintf(const char *format, va_list ap)
{
	os_vprintf(format, ap);

	// return vfprintf(stdout, format, ap);
	return 0;
}
