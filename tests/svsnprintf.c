/*
 * Version 1.1
 *
 * Copyright 2021 Oleg Nemanov <lego12239@yandex.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#define WIDTH_ASTERISK -2
#define PRECISION_ASTERISK -2

enum pad_type {
	pad_with_space = 1,
	pad_with_zero
};

enum len_mod {
	len_mod_none = 0,
	len_mod_h = 1,
	len_mod_l = 2,
	len_mod_L = 3,
	len_mod_j = 4,
	len_mod_z = 5,
	len_mod_Z = 6,
	len_mod_t = 7,
	len_mod_hh = 0x11,
	len_mod_ll = 0x22,
};

enum conv_spec {
	conv_spec_none = 0,
	conv_spec_d,
	conv_spec_o,
	conv_spec_u,
	conv_spec_x,
	conv_spec_X,
	conv_spec_b,
	conv_spec_f,
	conv_spec_c,
	conv_spec_s,
	conv_spec_p,
	conv_spec_PERCENT
};

/* See _fmtspec_init() */
#define FMTSPEC_INITIAL {1, 0, 0, 0, 0, -1, 0, 0, NULL}

struct _fmtspec {
	enum pad_type pad_type;
	unsigned int alternate_form;
	unsigned int left_adjust;
	unsigned int use_plus_sign;
	int width;  /* -2 - for * */
	int precision;  /* -2 - for * */
	enum len_mod len_mod;
	enum conv_spec conv_spec;
	int (*conv_fun)(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap);
};


int svsnprintf(char *str, size_t size, const char *fmt, va_list ap);
static unsigned int _fmtspec_collect(const char *fmt, struct _fmtspec *fmtspec);
static int conv_int(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap);
static int conv_double(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap);
static int conv_char(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap);
static int conv_str(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap);
static int conv_PERCENT(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap);

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
 * W - width(optional), P - precision(optional), L - length(optional),
 * C - conversion.
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
int
ssnprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = svsnprintf(str, size, fmt, ap);
	va_end(ap);

	return ret;
}

int
svsnprintf(char *str, size_t size, const char *fmt, va_list ap)
{
	unsigned int len, total = 0;
	struct _fmtspec fmtspec = FMTSPEC_INITIAL;

	/* Treat str = NULL like a size = 0. */
	if (!str)
		size = 0;
	/* Set str to NULL to differentiate between initial size=0 and
	 * size=0 due to buf end.
	 */
	if (size == 0)
		str = NULL;
	else
		size--; /* save a space for \0 */

	if (fmt == NULL)
		goto end;

	while (*fmt != '\0') {
//		printf("'%c'", *fmt);
		if (*fmt == '%') {
			len = _fmtspec_collect(fmt, &fmtspec);
//			printf("[%d]", len);
			if (fmtspec.conv_spec == conv_spec_none) {
				char *__str = "<-ERRSPEC";
				total += len;
				for (; len; len--) {
					if (size) {
						*str = *fmt;
						str++;
						size--;
					}
					fmt++;
				}
				len = 9;
				total += len;
				for (; len; len--) {
					if (size) {
						*str = *__str;
						str++;
						size--;
					}
					__str++;
				}
				goto end;
			} else {
				fmt += len;
				len = fmtspec.conv_fun(&str, &size, &fmtspec, ap);
				if (len < 0)
					return len;
				total += len;
			}
		} else {
			if (size) {
				*str = *fmt;
				str++;
				size--;
			}
			fmt++;
			total++;
		}
	}

end:
	if (str)
		*str = '\0';
	return total;
}

static void
_fmtspec_init(struct _fmtspec *fmtspec)
{
	fmtspec->pad_type = 1;
	fmtspec->alternate_form = 0;
	fmtspec->left_adjust = 0;
	fmtspec->use_plus_sign = 0;
	fmtspec->width = 0;
	fmtspec->precision = -1;
	fmtspec->len_mod = 0;
	fmtspec->conv_spec = 0;
	fmtspec->conv_fun = NULL;
}

static unsigned int
_fmtspec_collect(const char *fmt, struct _fmtspec *fmtspec)
{
	const char *start = fmt;
	/* 0 - flag
	 * 1 - width start(may be *)
	 * 2 - width
	 * 3 - precision marker
	 * 4 - precision start(may be *)
	 * 5 - precision
	 * 6 - length modifier
	 * 7 - conversion specifier
	 * 8 - happy end
	 * 9 - unknown conversion specification
	 */
	int state = 0;

	/* Error! Shouldn't happend */
	assert(fmt != NULL);
	assert(*fmt != '\0');
	fmt++;
	_fmtspec_init(fmtspec);
	while ((*fmt != '\0') && (state < 8)) {
//		fprintf(stderr, "state %d '%c'\n", state, *fmt);
		switch (state) {
		case 0:
			switch (*fmt) {
			case '0':
				if (!fmtspec->left_adjust)
					fmtspec->pad_type = pad_with_zero;
				break;
			case '#':
			case '-':
				fmtspec->left_adjust = 1;
				fmtspec->pad_type = pad_with_space;
				break;
			case ' ':
			case '+':
				fmtspec->use_plus_sign = 1;
				break;
			case '\'':
			case 'I':
				break;
			default:
				fmt--;
				state = 1;
				break;
			}
			break;
		case 1:
			if (*fmt == '*') {
				fmtspec->width = WIDTH_ASTERISK;
				state = 3;
			} else if ((*fmt >= '0') && (*fmt <= '9')) {
				fmtspec->width = *fmt - '0';
				state = 2;
			} else {
				fmt--;
				state = 3;
			}
			break;
		case 2:
			if ((*fmt < '0') || (*fmt > '9')) {
				state = 3;
				fmt--;
				break;
			}
			fmtspec->width = fmtspec->width * 10 + (*fmt - '0');
			break;
		case 3:
			if (*fmt == '.') {
				fmtspec->precision = 0;
				state = 4;
			} else {
				fmt--;
				state = 6;
			}
			break;
		case 4:
			if (*fmt == '*') {
				fmtspec->precision = PRECISION_ASTERISK;
				state = 6;
			} else if ((*fmt >= '0') && (*fmt <= '9')) {
				fmtspec->precision = *fmt - '0';
				state = 5;
			} else {
				fmt--;
				state = 6;
			}
			break;
		case 5:
			if ((*fmt < '0') || (*fmt > '9')) {
				state = 6;
				fmt--;
				break;
			}
			fmtspec->precision = fmtspec->precision * 10 + (*fmt - '0');
			break;
		case 6:
//			printf("state 6 %c\n", *fmt);
			switch (*fmt) {
			case 'h':
				fmtspec->len_mod = (fmtspec->len_mod << 4) + len_mod_h;
				break;
			case 'l':
				fmtspec->len_mod = (fmtspec->len_mod << 4) + len_mod_l;
				break;
			case 'L':
			case 'j':
			case 'z':
			case 'Z':
			case 't':
				break;
			default:
				state = 7;
				fmt--;
				break;
			}
			break;
		case 7:
//			printf("state 7 %c\n", *fmt);
			switch (*fmt) {
			case 'd':
			case 'i':
				fmtspec->conv_spec = conv_spec_d;
				fmtspec->conv_fun = conv_int;
				break;
			case 'b':
				fmtspec->conv_spec = conv_spec_b;
				fmtspec->conv_fun = conv_int;
				break;
			case 'o':
				fmtspec->conv_spec = conv_spec_o;
				fmtspec->conv_fun = conv_int;
				break;
			case 'u':
				fmtspec->conv_spec = conv_spec_u;
				fmtspec->conv_fun = conv_int;
				break;
			case 'x':
				fmtspec->conv_spec = conv_spec_x;
				fmtspec->conv_fun = conv_int;
				break;
			case 'X':
				fmtspec->conv_spec = conv_spec_X;
				fmtspec->conv_fun = conv_int;
				break;
			case 'f':
				fmtspec->conv_spec = conv_spec_f;
				fmtspec->conv_fun = conv_double;
				break;
			case 'c':
				fmtspec->conv_spec = conv_spec_c;
				fmtspec->conv_fun = conv_char;
				break;
			case 's':
				fmtspec->conv_spec = conv_spec_s;
				fmtspec->conv_fun = conv_str;
				break;
			case 'p':
				fmtspec->conv_spec = conv_spec_p;
				fmtspec->conv_fun = conv_int;
				break;
			case '%':
				fmtspec->conv_spec = conv_spec_PERCENT;
				fmtspec->conv_fun = conv_PERCENT;
				break;
			default:
				state = 9;
//				fmt--;
				break;
			}
			if (state != 9)
				state = 8;
			break;
		}
		fmt++;
	}

	return (fmt - start);
}

static int
conv_int(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap)
{
	char *conv = "0123456789abcdef";
	unsigned int digit, len = 0, num_len = 0, off, base = 10, is_msign = 0;
	int ppad_len, wpad_len;
	long long int _num;
	unsigned long long int num, num_copy;
	char pad_c;
	char *__str;

	if (fmtspec->width == WIDTH_ASTERISK) {
		fmtspec->width = va_arg(ap, int);
		if (fmtspec->width < 0)
			fmtspec->width = -fmtspec->width;
	}
	if (fmtspec->precision == PRECISION_ASTERISK) {
		fmtspec->precision = va_arg(ap, int);
		if (fmtspec->precision < 0)
			fmtspec->precision = -fmtspec->precision;
	}

	switch (fmtspec->conv_spec) {
	case conv_spec_d:
	case conv_spec_u:
		base = 10;
		break;
	case conv_spec_b:
		base = 2;
		fmtspec->use_plus_sign = 0;
		break;
	case conv_spec_o:
		base = 8;
		fmtspec->use_plus_sign = 0;
		break;
	case conv_spec_p:
		fmtspec->alternate_form = 1;
	case conv_spec_x:
		base = 16;
		fmtspec->use_plus_sign = 0;
		break;
	case conv_spec_X:
		base = 16;
		fmtspec->use_plus_sign = 0;
		conv = "0123456789ABCDEF";
		break;
	default:
		__str = "conv_int: wrong fmtspec->conv_spec\n";
//		write(2, __str, 35);
		write(2, __str, strlen(__str));
		exit(1);
	}

	switch (fmtspec->conv_spec) {
	case conv_spec_d:
		switch (fmtspec->len_mod) {
		case len_mod_none:
			_num = va_arg(ap, int);
			break;
		case len_mod_h:
			_num = (short int)va_arg(ap, int);
			break;
		case len_mod_hh:
			_num = (char)va_arg(ap, int);
			break;
		case len_mod_l:
			_num = va_arg(ap, long int);
			break;
		case len_mod_ll:
			_num = va_arg(ap, long long int);
			break;
		default:
			break;
		}
		if (_num < 0) {
			is_msign = 1;
			_num *= -1;
		}
		num = _num;
		break;
	case conv_spec_p:
		num = (unsigned long long int)va_arg(ap, void*);
		break;
	default:
		switch (fmtspec->len_mod) {
		case len_mod_none:
			num = va_arg(ap, unsigned int);
			break;
		case len_mod_h:
			num = (unsigned short int)va_arg(ap, unsigned int);
			break;
		case len_mod_hh:
			num = (unsigned char)va_arg(ap, unsigned int);
			break;
		case len_mod_l:
			num = va_arg(ap, unsigned long int);
			break;
		case len_mod_ll:
			num = va_arg(ap, unsigned long long int);
			break;
		default:
			break;
		}
		break;
	}

	/* Calculate a length of a number */
	num_copy = num;
	do {
		num_copy = num_copy / base;
		num_len++;
	} while (num_copy);

	/* Calculate paddings length */
	ppad_len = fmtspec->precision - num_len;
	if (ppad_len < 0)
		ppad_len = 0;
	wpad_len = fmtspec->width - num_len -
	  (is_msign || fmtspec->use_plus_sign) - ppad_len;
	if (wpad_len < 0)
		wpad_len = 0;
	/* Convert a wpad to ppad if pad_type is pad_with_zero */
	switch (fmtspec->pad_type) {
	case pad_with_zero:
		pad_c = '0';
		if (wpad_len > 0)
			ppad_len += wpad_len;
		wpad_len = 0;
		break;
	default:
		pad_c = ' ';
		break;
	}

	/* width padding */
	if ((wpad_len > 0) && (!fmtspec->left_adjust)) {
		len += wpad_len;
		for (; wpad_len && *size; wpad_len--, *str += 1, *size -= 1)
			**str = pad_c;
	}

	/* A sign */
	if (is_msign) {
		if (*size) {
			**str = '-';
			*str += 1;
			*size -= 1;
		}
		len++;
	} else if (fmtspec->use_plus_sign) {
		if (*size) {
			**str = '+';
			*str += 1;
			*size -= 1;
		}
		len++;
	}

	/* alternate form */
	if (fmtspec->alternate_form) {
		switch (fmtspec->conv_spec) {
		case conv_spec_p:
			len += 2;
			if (*size) {
				**str = '0';
				*str += 1;
				*size -= 1;
			}
			if (*size) {
				**str = 'x';
				*str += 1;
				*size -= 1;
			}
			break;
		default:
			break;
		}
	}
	/* precision padding */
	if (ppad_len > 0) {
		len += ppad_len;
		for (; ppad_len && *size; ppad_len--, *str += 1, *size -= 1)
			**str = '0';
	}

	/* A number */
	len += num_len;
	off = 0;
	do {
		digit = num % base;
		num = num / base;
		if (*size) {
			/* do not write digit if there is no empty space in str */
			if (num_len <= *size) {
				*(*str + num_len - 1) = conv[digit];
				off++;
				*size -= 1;
			}
		}
		num_len--;
	} while (num);
	*str += off;

	/* width padding */
	if ((wpad_len > 0) && (fmtspec->left_adjust)) {
		len += wpad_len;
		for (; wpad_len && *size; wpad_len--, *str += 1, *size -= 1)
			**str = pad_c;
	}

	return len;
}

static int
conv_double(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap)
{
	char *conv = "0123456789abcdef";
	unsigned int digit, len = 0, off, is_msign = 0;
	int ipart_len = 0, fpart_len = 0, rmult = 1, zpad_len = 0, wpad_len, i;
	long long int num_ipart, num_fpart, num_copy;
	double num;
	char pad_c;

	if (fmtspec->width == WIDTH_ASTERISK) {
		fmtspec->width = va_arg(ap, int);
		if (fmtspec->width < 0)
			fmtspec->width = -fmtspec->width;
	}
	if (fmtspec->precision == PRECISION_ASTERISK) {
		fmtspec->precision = va_arg(ap, int);
		if (fmtspec->precision < 0)
			fmtspec->precision = -fmtspec->precision;
	}

	/* separate a number to integer and fractional parts */
	num = (double)va_arg(ap, double);
	if (num < 0) {
		is_msign = 1;
		num *= -1;
	}
	num_ipart = (long long int)num;
	num = num - num_ipart;

	fpart_len = fmtspec->precision;
	if (fpart_len == -1)
		fpart_len = 6;

	/* round a number */
	for(i = 0; i < fpart_len; i++)
		rmult *= 10;
	num += 0.5/rmult;

	/* continue to separate a number to integer and fractional parts */
	num_ipart += (int)num;
	num -= (int)num;
	num_fpart = (int)(num*rmult);

	/* Calculate a length of a number */
	num_copy = num_ipart;
	do {
		num_copy = num_copy / 10;
		ipart_len++;
	} while (num_copy);

	/* Calculate paddings length */
	wpad_len = fmtspec->width - ipart_len - fpart_len - (fpart_len ? 1 : 0) -
	  (is_msign || fmtspec->use_plus_sign);
	if (wpad_len < 0)
		wpad_len = 0;
	/* Convert a wpad to ppad if pad_type is pad_with_zero */
	switch (fmtspec->pad_type) {
	case pad_with_zero:
		pad_c = '0';
		if (wpad_len > 0)
			zpad_len += wpad_len;
		wpad_len = 0;
		break;
	default:
		pad_c = ' ';
		break;
	}

	/* width padding */
	if ((wpad_len > 0) && (!fmtspec->left_adjust)) {
		len += wpad_len;
		for (; wpad_len && *size; wpad_len--, *str += 1, *size -= 1)
			**str = pad_c;
	}

	/* A sign */
	if (is_msign) {
		if (*size) {
			**str = '-';
			*str += 1;
			*size -= 1;
		}
		len++;
	} else if (fmtspec->use_plus_sign) {
		if (*size) {
			**str = '+';
			*str += 1;
			*size -= 1;
		}
		len++;
	}

	/* padding with zeroes */
	if (zpad_len > 0) {
		len += zpad_len;
		for (; zpad_len && *size; zpad_len--, *str += 1, *size -= 1)
			**str = '0';
	}

	/* An integer number part */
	len += ipart_len;
	off = 0;
	do {
		digit = num_ipart % 10;
		num_ipart = num_ipart / 10;
		if (*size) {
			/* do not write digit if there is no empty space in str */
			if (ipart_len <= *size) {
				*(*str + ipart_len - 1) = conv[digit];
				off++;
				*size -= 1;
			}
		}
		ipart_len--;
	} while (num_ipart);
	*str += off;

	/* A fractional number part */
	if (fpart_len) {
		if (*size) {
			**str = '.';
			*str += 1;
			*size -= 1;
		}
		len += fpart_len + 1;
		off = 0;
		do {
			digit = num_fpart % 10;
			num_fpart = num_fpart / 10;
			if (*size) {
				/* do not write digit if there is no empty space in str */
				if (fpart_len <= *size) {
					*(*str + fpart_len - 1) = conv[digit];
					off++;
					*size -= 1;
				}
			}
			fpart_len--;
		} while (num_fpart);
		*str += off;
	}

	/* width padding */
	if ((wpad_len > 0) && (fmtspec->left_adjust)) {
		len += wpad_len;
		for (; wpad_len && *size; wpad_len--, *str += 1, *size -= 1)
			**str = pad_c;
	}

	return len;
}


static int
conv_char(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap)
{
	char c;

	c = (char)va_arg(ap, int);
	if (*size) {
		**str = c;
		*str += 1;
		*size -= 1;
	}

	return 1;
}

static int
conv_str(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap)
{
	int len = 0, maxlen, i, str_len, wpad_len;
	char *buf, *ptr;

	if (fmtspec->width == WIDTH_ASTERISK) {
		fmtspec->width = va_arg(ap, int);
		if (fmtspec->width < 0)
			fmtspec->width = -fmtspec->width;
	}
	if (fmtspec->precision == PRECISION_ASTERISK) {
		fmtspec->precision = va_arg(ap, int);
		if (fmtspec->precision < 0)
			fmtspec->precision = -fmtspec->precision;
	}

	buf = va_arg(ap, void*);
	if (!buf)
		buf = "(null)";
	maxlen = fmtspec->precision;

	/* calculate a length of a string */
	ptr = buf;
	for(i = 0; *ptr && (i != maxlen); i++, ptr++);
	str_len = ptr - buf;

	/* calculate padding length */
	wpad_len = fmtspec->width - str_len;
	if (wpad_len < 0)
		wpad_len = 0;

	/* width padding */
	if ((wpad_len > 0) && (!fmtspec->left_adjust)) {
		len += wpad_len;
		for (; wpad_len && *size; wpad_len--, *str += 1, *size -= 1)
			**str = ' ';
	}

	/* a string */
	len += str_len;
	for(i = 0; *size && (i < str_len); i++, *str += 1, *size -= 1)
		**str = buf[i];

	/* width padding */
	if ((wpad_len > 0) && (fmtspec->left_adjust)) {
		len += wpad_len;
		for (; wpad_len && *size; wpad_len--, *str += 1, *size -= 1)
			**str = ' ';
	}

	return len;
}

static int
conv_PERCENT(char **str, size_t *size, struct _fmtspec *fmtspec, va_list ap)
{
	if (*size) {
		**str = '%';
		*str += 1;
		*size -= 1;
	}
	return 1;
}

