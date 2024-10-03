#ifndef __EGETOPT_H__
#define __EGETOPT_H__

struct egetopt_opt {
	const char *name;
	int val;
};

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
int egetopt_parse_hsearch(int argc, char **argv, struct egetopt_opt opts[], int (*cb)(const struct egetopt_opt *odef, const char *oval, void *priv), void *priv);

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
int egetopt_parse(int argc, char **argv, int (*cb)(const char *oname, const char *oval, void *priv), void *priv);

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
const char* egetopt_strerror(int ecode);

#endif /* __EGETOPT_H__ */
