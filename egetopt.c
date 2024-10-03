/*
 * Version 0.9
 *
 * Copyright 2024 Oleg Nemanov <lego12239@yandex.ru>
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
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "egetopt.h"
#include <search.h>


struct egetopt_opts_hsearch {
	struct hsearch_data *htab;
	int (*cb)(const struct egetopt_opt *odef, const char *oval, void *priv);
	void *priv;
};


static int
egetopt_parse_hsearch_cb(const char *oname, const char *oval, void *priv)
{
	struct egetopt_opts_hsearch *eopts = priv;
	ENTRY hentry, *hentry_ret;

	hentry.key = (char*)oname;
	hentry.data = NULL;
	if (hsearch_r(hentry, FIND, &hentry_ret, eopts->htab) == 0)
		return eopts->cb(NULL, oname, eopts->priv);
	else
		return eopts->cb(hentry_ret->data, oval, eopts->priv);
}

/*
 * Parse command line options from argv.
 *
 * prms:
 *  argc - arguments count
 *  argv - arguments array (argv[0] should be a program name)
 *  opts - options definitions
 *  cb   - a callback to call on each option found
 *  priv - a private data which will be passed to a callback untouched
 *
 * ret:
 *  ARGV_CNT  - a count of arguments remains in argv (non-option arguments)
 *
 * This function is identical to egetopt_parse(), except:
 * - hsearch_r() is used to get an option definition from specified opts[]
 * - cb() is got as first argument an option definition from opts[]
 * - if an option isn't found in opts[], then cb odef is NULL and oval is
 *   set to this option name
 *
 * See egetopt_parse() for further info.
 */
int
egetopt_parse_hsearch(int argc, char **argv, struct egetopt_opt opts[],
int (*cb)(const struct egetopt_opt *odef, const char *oval, void *priv),
void *priv)
{
	struct hsearch_data htab;
	struct egetopt_opts_hsearch eopts;
	ENTRY hentry, *hentry_ret;
	int i;

	for (i = 0; opts[i].name != NULL; i++);
	memset(&htab, 0, sizeof(htab));
	if (hcreate_r(1.25*i, &htab) == 0)
		return -10;

	for (i = 0; opts[i].name != NULL; i++) {
		hentry.key = (char*)opts[i].name;
		hentry.data = &opts[i];
		if (hsearch_r(hentry, ENTER, &hentry_ret, &htab) == 0) {
			hdestroy_r(&htab);
			return -11;
		}
	}

	eopts.cb = cb;
	eopts.htab = &htab;
	eopts.priv = priv;
	i = egetopt_parse(argc, argv, egetopt_parse_hsearch_cb, &eopts);
	hdestroy_r(&htab);
	return i;
}

/*
 * Parse command line options from argv.
 *
 * prms:
 *  argc - arguments count
 *  argv - arguments array (argv[0] should be a program name)
 *  cb   - a callback to call on each option found
 *  priv - a private data which will be passed to a callback untouched
 *
 * ret:
 *  ARGV_CNT  - a count of arguments remains in argv (non-option arguments)
 *
 * cb is called on each found option. A second argument to it is an option
 * value. This is a possible value if an option. egetopt_parse() doesn't know
 * is this option require a value or not. But it parses argv as if every option
 * require a value. And callback must tell through it return code If this
 * option is an option with a value or not. A callback must return 0 for option
 * without a value and 1 for option with a value. If cb returns 1, then an
 * argument with this option is removed from argv. If cb returns 0, then
 * this argument is not removed from argv and parsed as usual in the next
 * iteration. For short option, oname is a pointer to an internal egetopt_parse
 * local buffer; so, you need to strdup it if you want to use it later.
 *
 * Option and their values are removed from argv and, after egetopt_parse
 * return, argv contains only non-option arguments.
 */
int
egetopt_parse(int argc, char **argv,
int (*cb)(const char *oname, const char *oval, void *priv),
void *priv)
{
	char *olist = NULL, *arg, *pos, *oname, *oval, sopt[2] = {'-', 'o'};
	int i, j, argv_idx = -1;

	i = 1;
	while ((i < argc) || olist) {
		arg = argv[i];
		oname = NULL;
		if (olist) {
			oname = sopt;
			sopt[1] = olist[0];
			olist++;
			if (olist[0] != '\0') {
				oval = olist;
			} else {
				if (i < argc) {
					oval = argv[i];
					argv_idx = i;
				} else {
					oval = NULL;
				}
				olist = NULL;
			}
		} else {
			if (arg[0] == '-') {
				if (arg[1] == '-') {
					if (arg[2] == '\0') {
						argv[i] = NULL;
						break;
					} else {
						oname = arg;
						argv[i] = NULL;
						pos = strchr(arg, '=');
						if (pos) {
							*pos = '\0';
							oval = pos + 1;
						} else {
							if ((i+1) < argc) {
								oval = argv[i+1];
								argv_idx = i + 1;
							} else {
								oval = NULL;
							}
						}
//						printf("long: %s=%s\n", oname, oval);
					}
				} else {
					olist = arg + 1;
					argv[i] = NULL;
//					printf("opt list: %s\n", arg);
				}
			}
			i++;
		}

		if (oname) {
//			printf("%s=%s\n", oname, oval);
			if (cb(oname, oval, priv)) {
				olist = NULL;
				if (argv_idx >= 0) {
					argv[argv_idx] = NULL;
					i++;
				}
			}
			argv_idx = -1;
		}
	}

	/* Move remain arguments to the head of the array. */
	for (i = 0, j = 1; j < argc; i++, j++) {
		if (argv[i] == NULL) {
			while ((j < argc) && (argv[j] == NULL))
				j++;
			/* i shouldn't incremented if it's already at NULL. */
			if (j >= argc)
				break;
			argv[i] = argv[j];
			argv[j] = NULL;
		}
	}

	/* i must contain the args count. */
	while ((i < argc) && (argv[i] != NULL))
		i++;

	return i;
}

/*
 * Return a pointer to a error string for specified error code.
 *
 * prms:
 *  code - the error code from egetopt_* routines.
 *
 * ret:
 *  MSG  - an error message corresponding to the code
 *
 * DO NOT free returned pointer! It is a static string.
 */
const char*
egetopt_strerror(int ecode)
{
        switch (ecode) {
        case 0:
                return "success";
        case -10:
                return "hcreate_r() error";
        case -11:
                return "hsearch_r() error";
        default:
                return "unknown error code";
        }

        return "unknown error code";
}

