#ifndef __SVSNPRINTF_H__
#define __SVSNPRINTF_H__

#include <stdarg.h>

/*
 * snprintf() and vsnprintf() replacement.
 *  str - a destination buffer
 *  size - a size of str
 *  fmt - format string
 *  ... - values for fmt
 *
 * return:
 *  >=0 - the length of resulting string (wihout \0)
 *
 * Place a formatted string in str buffer with maximum size of size.
 * Conversion specification: %FW.PLC. Where F - flags(optional),
 * W - width(optional), P - precision(optional), L - length(optional), C - conversion.
 * Supported conversion specifications:
 *  flags:
 *    0, -, +
 *  width:
 *    NUMBER or *
 *  precision:
 *    NUMBER or *
 *  length modifier:
 *    hh, h, l, ll
 *  conversion specifier:
 *    d, i, o, u, x, X, b(binary form), f, c, s, p
 *
 * If str or/and size is 0, then no data is written to a destination buffer;
 * only needed buffer length(without the terminating \0 byte) is returned.
 * If fmt is NULL, then \0 byte is written to a destination buffer(if there is
 * space for it according to a size value) and 0 is returned.
 * If unknown conversion specifier is found, then it is written to a destination
 * buffer with postfix "<-ERRSPEC" and no further processing is done to prevent
 * a memory error by misinterpretation of function arguments.
 */
int ssnprintf(char *str, size_t size, const char *fmt, ...);
int svsnprintf(char *str, size_t size, const char *fmt, va_list ap);

#endif  /* __SVSNPRINTF_H__ */
