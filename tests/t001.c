#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include "tests.h"
#include "../egetopt.c"


struct alist {
	const char *args[100][2];
	int cnt;
};


struct alist alist;


void
init(void)
{
}

void
pretest(int idx)
{
}

void
posttest(int idx)
{
}

void
deinit(void)
{
}

void
alist_add(const char *oname, const char *oval)
{
	if (alist.cnt == 100)
		TERR_EXIT("alist args count is 100!");

	if (oname) {
		alist.args[alist.cnt][0] = strdup(oname);
		if (!alist.args[alist.cnt][0])
			TERR_EXIT("alist_add: no free mem");
	} else {
		alist.args[alist.cnt][0] = NULL;
	}

	if (oval) {
		alist.args[alist.cnt][1] = strdup(oval);
		if (!alist.args[alist.cnt][1])
			TERR_EXIT("alist_add: no free mem");
	} else {
		alist.args[alist.cnt][1] = NULL;
	}
	alist.cnt++;
}

void
alist_clean(void)
{
	int i;

	for (i = 0; i < alist.cnt; i++) {
		if (alist.args[i][0])
			free((char*)alist.args[i][0]);
		if (alist.args[i][1])
			free((char*)alist.args[i][1]);
	}
	alist.cnt = 0;
}

int
alist_is_eq(int cnt, ...)
{
	va_list ap;
	char *s;
	int i, j;

	va_start(ap, cnt);
	for (i = 0; i < cnt; i++) {
		for (j = 0; j < 2; j++) {
			s = va_arg(ap, char*);
			if (((s == NULL) || (alist.args[i][j] == NULL)) &&
			    (s != alist.args[i][j]))
					return 0;
			if (s == NULL)
				continue;
			if (strcmp(s, alist.args[i][j]) != 0)
				return 0;
		}
	}
	va_end(ap);

	return 1;
}

void
alist_dump(void)
{
	int i;

	for (i = 0; i < alist.cnt; i++) {
		printf("%c %d: %s=%s", i ? ',' : ' ', i, alist.args[i][0], alist.args[i][1]);
	}
	printf("\n");
}

char**
argv_mk(int argc, ...)
{
	va_list ap;
	char **argv, *s;
	int i;

	argv = malloc(sizeof(*argv) * argc);
	va_start(ap, argc);
	for (i = 0; i < argc; i++) {
		s = va_arg(ap, char*);
		argv[i] = strdup(s);
		if (!argv[i])
			TERR_EXIT("argv_mk: no free mem");
	}
	va_end(ap);

	return argv;
}

void
argv_free(int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++)
		if (argv[i])
			free(argv[i]);
	free(argv);
}

int
argv_is_eq(int argc, char **argv, ...)
{
	va_list ap;
	char *s;
	int i;

	va_start(ap, argv);
	for (i = 0; i < argc; i++) {
		s = va_arg(ap, char*);
		if (((s == NULL) || (argv[i] == NULL)) &&
		    (s != argv[i]))
				return 0;
		if (s == NULL)
			continue;
		if (strcmp(s, argv[i]) != 0)
			return 0;
	}
	va_end(ap);

	return 1;
}

void
argv_dump(int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		printf("%c %s", i ? ',' : ' ', argv[i]);
	}
	printf("\n");
}

int
cb1(const char *oname, const char *oval, void *priv)
{
	int *n = priv;

	alist_add(oname, oval);
	*n = alist.cnt;

	return 0;
}

int
cb2(const char *oname, const char *oval, void *priv)
{
	int *n = priv;

	if ((strcmp(oname, "-d") == 0) ||
	    (strcmp(oname, "--debug") == 0) ||
	    (strcmp(oname, "-l") == 0) ||
	    (strcmp(oname, "--log-level") == 0)) {
		alist_add(oname, oval);
		return 1;
	}
	alist_add(oname, NULL);

	return 0;
}

int
cb_hsearch(const struct egetopt_opt *odef, const char *oval, void *priv)
{
	int *n = priv;

	if (odef == NULL) {
		alist_add(oval, NULL);
		*n = alist.cnt;
		return 0;
	}

	switch (odef->val) {
	case 'd':
		alist_add("-d", oval);
		break;
	case 'l':
		alist_add("-l", oval);
		break;
	}

	*n = alist.cnt;
	return 1;
}

int
test0_0_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	argc = 0;
	argv = argv_mk(argc);

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != argc)
		TERR("ret must be %d instead of %d", argc, ret);
	if (!argv_is_eq(argc, argv)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 0)
		TERR("alist.cnt must be 0 instead of: %d", alist.cnt);

	argv_free(argc, argv);

	return 1;
}

int
test0_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	argc = 1;
	argv = argv_mk(argc, "prg");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != argc)
		TERR("ret must be %d instead of %d", argc, ret);
	if (!argv_is_eq(argc, argv, "prg")) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 0)
		TERR("alist.cnt must be 0 instead of: %d", alist.cnt);

	argv_free(argc, argv);

	return 1;
}

int
test0_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	argc = 2;
	argv = argv_mk(argc, "prg", "arg1");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != argc)
		TERR("ret must be %d instead of %d", argc, ret);
	if (!argv_is_eq(argc, argv, "prg", "arg1")) {
		argv_dump(argc, argv);
		TERR("argv is not (prg, arg1)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 0)
		TERR("alist.cnt must be 0 instead of: %d", alist.cnt);

	argv_free(argc, argv);

	return 1;
}

int
test0_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	argc = 3;
	argv = argv_mk(argc, "prg", "arg1", "arg2");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != argc)
		TERR("ret must be %d instead of %d", argc, ret);
	if (!argv_is_eq(argc, argv, "prg", "arg1", "arg2")) {
		argv_dump(argc, argv);
		TERR("argv is not (prg, arg1, arg2)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 0)
		TERR("alist.cnt must be 0 instead of: %d", alist.cnt);

	argv_free(argc, argv);

	return 1;
}

int
test1_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 2;
	argv = argv_mk(argc, "prg", "-a");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 1)
		TERR("alist.cnt must be 1 instead of: %d", alist.cnt);
	if (!alist_is_eq(1, "-a", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test1_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 3;
	argv = argv_mk(argc, "prg", "-a", "-v");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", "-v", "-v", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, -v, -v, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test1_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 4;
	argv = argv_mk(argc, "prg", "-a", "-v", "-d");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", "-v", "-v", "-d", "-d", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, -v, -v, -d, -d, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test2_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 2;
	argv = argv_mk(argc, "prg", "-av");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", "v", "-v", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, v, -v, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test2_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 2;
	argv = argv_mk(argc, "prg", "-avd");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", "vd", "-v", "d", "-d", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, v, -v, d, -d, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test3_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 5;
	argv = argv_mk(argc, "prg", "some", "-a", "nonopts", "args");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 4)
		TERR("ret must be %d instead of %d", 4, ret);
	if (!argv_is_eq(argc, argv, "prg", "some", "nonopts", "args", NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 1)
		TERR("alist.cnt must be 1 instead of: %d", alist.cnt);
	if (!alist_is_eq(1, "-a", "nonopts")) {
		alist_dump();
		TERR("alist isn't equal to (-a, nonopts)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test3_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 6;
	argv = argv_mk(argc, "prg", "some", "-a", "nonopts", "-v", "args");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 4)
		TERR("ret must be %d instead of %d", 4, ret);
	if (!argv_is_eq(argc, argv, "prg", "some", "nonopts", "args", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", "nonopts", "-v", "args")) {
		alist_dump();
		TERR("alist isn't equal to (-a, nonopts, -v, args)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test3_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "some", "-a", "nonopts", "-v", "args", "-d", "1");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "some", "nonopts", "args", "1", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", "nonopts", "-v", "args", "-d", "1")) {
		alist_dump();
		TERR("alist isn't equal to (-a, nonopts, -v, args, -d, 1)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test4_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 6;
	argv = argv_mk(argc, "prg", "s", "o", "-ad", "m", "e");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", "d", "-d", "m")) {
		alist_dump();
		TERR("alist isn't equal to (-a, d, -d, m)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test4_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 7;
	argv = argv_mk(argc, "prg", "s", "-a", "o", "-d", "m", "e");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", "o", "-d", "m")) {
		alist_dump();
		TERR("alist isn't equal to (-a, o, -d, m)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test4_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 6;
	argv = argv_mk(argc, "prg", "-a", "-d", "-b", "-D", "-e");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 5)
		TERR("alist.cnt must be 5 instead of: %d", alist.cnt);
	if (!alist_is_eq(5, "-a", "-d", "-d", "-b", "-b", "-D", "-D", "-e", "-e", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, -d, -d, -b, -b, -D, -D, -e, -e, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test4_3(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 3;
	argv = argv_mk(argc, "prg", "-adbD", "-e");

	ret = egetopt_parse(argc, argv, cb1, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != priv)
		TERR("priv must be %d: instead of %d",
		  alist.cnt, priv);
	if (alist.cnt != 5)
		TERR("alist.cnt must be 5 instead of: %d", alist.cnt);
	if (!alist_is_eq(5, "-a", "dbD", "-d", "bD", "-b", "D", "-D", "-e", "-e", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, dbD, -d, bD, -b, D, -D, -e, -e, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test5_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 6;
	argv = argv_mk(argc, "prg", "-a", "-d", "-b", "-D", "-e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 4)
		TERR("alist.cnt must be 4 instead of: %d", alist.cnt);
	if (!alist_is_eq(4, "-a", NULL, "-d", "-b", "-D", NULL, "-e", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -d, -b, -D, NULL, -e, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test5_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 3;
	argv = argv_mk(argc, "prg", "-a", "-l");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", NULL, "-l", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test5_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 2;
	argv = argv_mk(argc, "prg", "-al");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", NULL, "-l", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test6_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 3;
	argv = argv_mk(argc, "prg", "-a", "--log-level");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", NULL, "--log-level", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --log-level, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 4;
	argv = argv_mk(argc, "prg", "-a", "-l0123", "-b");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "-a", "o", "-l0123", "m", "-b", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 3;
	argv = argv_mk(argc, "prg", "-al0123", "-b");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_3(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 7;
	argv = argv_mk(argc, "prg", "s", "-al0123", "o", "-b", "m", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_4(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 5;
	argv = argv_mk(argc, "prg", "-a", "-l", "0123", "-b");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_5(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 9;
	argv = argv_mk(argc, "prg", "s", "-a", "o", "-l", "0123", "m", "-b", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_6(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 4;
	argv = argv_mk(argc, "prg", "-al", "0123", "-b");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test7_7(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "-al", "0123", "o", "-b", "m", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "-l", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -l, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test8_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 4;
	argv = argv_mk(argc, "prg", "-a", "--log-level=0123", "-b");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "--log-level", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --log-level, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test8_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "-a", "o", "--log-level=0123", "m", "-b", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "--log-level", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --log-level, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test8_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "--unknown", "o", "--log-level=0123", "m", "-b", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "--unknown", NULL, "--log-level", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (--unknown, NULL, --log-level, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test8_3(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "--unknown=value", "o", "--log-level=0123", "m", "-b", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "--unknown", NULL, "--log-level", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (--unknown, NULL, --log-level, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test9_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 5;
	argv = argv_mk(argc, "prg", "-a", "--log-level", "0123", "-b");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "--log-level", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --log-level, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test9_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 9;
	argv = argv_mk(argc, "prg", "s", "-a", "o", "--log-level", "0123", "m", "-b", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 3)
		TERR("alist.cnt must be 3 instead of: %d", alist.cnt);
	if (!alist_is_eq(3, "-a", NULL, "--log-level", "0123", "-b", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --log-level, 0123, -b, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test10_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 6;
	argv = argv_mk(argc, "prg", "s", "-ad456", "o", "m", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 5)
		TERR("ret must be %d instead of %d", 5, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", NULL, "-d", "456")) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -d, 456)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 2;
	argv = argv_mk(argc, "prg", "--");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 1)
		TERR("ret must be %d instead of %d", 1, ret);
	if (!argv_is_eq(argc, argv, "prg", NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 0)
		TERR("alist.cnt must be 0 instead of: %d", alist.cnt);
	if (!alist_is_eq(0)) {
		alist_dump();
		TERR("alist isn't equal to ()");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_1(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "--", "o", "-a", "m", "-l1", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 7)
		TERR("ret must be %d instead of %d", 7, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "-a", "m", "-l1", "e", NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 0)
		TERR("alist.cnt must be 0 instead of: %d", alist.cnt);
	if (!alist_is_eq(0)) {
		alist_dump();
		TERR("alist isn't equal to ()");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_2(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 8;
	argv = argv_mk(argc, "prg", "s", "-a", "--", "o", "m", "-l1", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 6)
		TERR("ret must be %d instead of %d", 6, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "-l1", "e", NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 1)
		TERR("alist.cnt must be 1 instead of: %d", alist.cnt);
	if (!alist_is_eq(1, "-a", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_3(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 9;
	argv = argv_mk(argc, "prg", "s", "-a", "--version", "--", "o", "m", "-l1", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 6)
		TERR("ret must be %d instead of %d", 6, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "-l1", "e", NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 2)
		TERR("alist.cnt must be 2 instead of: %d", alist.cnt);
	if (!alist_is_eq(2, "-a", NULL, "--version", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --version, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_4(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 13;
	argv = argv_mk(argc, "prg", "-a", "s", "-d", "-b", "--version", "-c", "--", "-d", "o", "m", "-l1", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 7)
		TERR("ret must be %d instead of %d", 7, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "-d", "o", "m", "-l1", "e", NULL, NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 4)
		TERR("alist.cnt must be 4 instead of: %d", alist.cnt);
	if (!alist_is_eq(4, "-a", NULL, "-d", "-b", "--version", NULL, "-c", NULL)) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -d, -b, --version, NULL, -c, NULL)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_5(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 15;
	argv = argv_mk(argc, "prg", "-a", "s", "-d", "o", "-b", "m", "--version", "e", "-l", "--", "-d", "o", "m", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 6)
		TERR("ret must be %d instead of %d", 6, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "m", "e", "m", "e", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 6)
		TERR("alist.cnt must be 6 instead of: %d", alist.cnt);
	if (!alist_is_eq(6, "-a", NULL, "-d", "o", "-b", NULL, "--version", NULL, "-l", "--", "-d", "o")) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -d, o, -b, NULL, --version, NULL, -l, --, -d, o)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test11_6(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 15;
	argv = argv_mk(argc, "prg", "-a", "s", "-d", "o", "-b", "m", "--version", "e", "--log-level", "--", "-d", "o", "m", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 6)
		TERR("ret must be %d instead of %d", 6, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "m", "e", "m", "e", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 6)
		TERR("alist.cnt must be 6 instead of: %d", alist.cnt);
	if (!alist_is_eq(6, "-a", NULL, "-d", "o", "-b", NULL, "--version", NULL, "--log-level", "--", "-d", "o")) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, -d, o, -b, NULL, --version, NULL, --log-level, --, -d, o)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test12_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;

	alist_clean();
	argc = 15;
	argv = argv_mk(argc, "prg", "-a", "s", "--d", "o", "--b", "m", "--version", "e", "--log-level", "--", "-d", "o", "m", "e");

	ret = egetopt_parse(argc, argv, cb2, &priv);
	if (ret != 7)
		TERR("ret must be %d instead of %d", 7, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "o", "m", "e", "m", "e", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 6)
		TERR("alist.cnt must be 6 instead of: %d", alist.cnt);
	if (!alist_is_eq(6, "-a", NULL, "--d", NULL, "--b", NULL, "--version", NULL, "--log-level", "--", "-d", "o")) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --d, NULL, --b, NULL, --version, NULL, --log-level, --, -d, o)");
	}

	argv_free(argc, argv);

	return 1;
}

int
test13_0(void)
{
	int ret = 1, priv = 0, argc;
	char **argv;
	struct egetopt_opt opts[] = {{"-d", 'd'}, {"--debug", 'd'}, {"-l", 'l'},
	  {"--log-level", 'l'}, {NULL, 0}};

	alist_clean();
	argc = 11;
	argv = argv_mk(argc, "prg", "-a", "s", "-d", "o", "-d1", "m", "--debug=2", "e", "--debug", "4");

	ret = egetopt_parse_hsearch(argc, argv, opts, cb_hsearch, &priv);
	if (ret != 4)
		TERR("ret must be %d instead of %d", 4, ret);
	if (!argv_is_eq(argc, argv, "prg", "s", "m", "e", NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		argv_dump(argc, argv);
		TERR("argv is not (prg)");
	}
	if (alist.cnt != 5)
		TERR("alist.cnt must be 5 instead of: %d", alist.cnt);
	if (!alist_is_eq(5, "-a", NULL, "-d", "o", "-d", "1", "-d", "2", "-d", "4")) {
		alist_dump();
		TERR("alist isn't equal to (-a, NULL, --d, o, -d, 1, -d, 2, -d , 4)");
	}

	argv_free(argc, argv);

	return 1;
}


struct test tests[] = {
	{test0_0_0, "0 opts, 0 args"},
	{test0_0, "0 opts"},
	{test0_1, "0 opts"},
	{test0_2, "0 opts"},
	{test1_0, "1 opts"},
	{test1_1, "2 opts"},
	{test1_2, "3 opts"},
	{test2_0, "2 opts, glued"},
	{test2_1, "3 opts, glued"},
	{test3_0, "1 opts, with non opts"},
	{test3_1, "2 opts, with non opts"},
	{test3_2, "3 opts, with non opts"},
	{test4_0, "2 opts, with non opts, glued"},
	{test4_1, "2 opts, with non opts"},
	{test4_2, "5 opts"},
	{test4_3, "5 opts, glued"},
	{test5_0, "5 opts, with vals"},
	{test5_1, "1 opts, with vals"},
	{test5_2, "1 opts, with vals, glued"},
	{test6_0, "1 long opts, with vals"},
	{test7_0, "3 opts, with vals"},
	{test7_1, "3 opts, with vals, with non opts"},
	{test7_2, "3 opts, with vals, glued"},
	{test7_3, "3 opts, with vals, with non opts, glued"},
	{test7_4, "3 opts, with vals(separated)"},
	{test7_5, "3 opts, with vals(separated), with non opts"},
	{test7_6, "3 opts, with vals(separated), glued"},
	{test7_7, "3 opts, with vals(separated), with non opts, glued"},
	{test8_0, "3 opts, with vals(=)"},
	{test8_1, "3 opts, with vals(=), with non opts"},
	{test8_2, "3 opts, with vals(=), with non opts"},
	{test8_3, "3 opts, with vals(=), with non opts"},
	{test9_0, "long opts, with vals(separated)"},
	{test9_1, "long opts, with vals(separated), with non opts"},
	{test10_0, "2 opts, with vals, glued"},
	{test11_0, "--"},
	{test11_1, "--, with opts, with vals, with non opts"},
	{test11_2, "--, with opts, with vals, with non opts"},
	{test11_3, "--, with opts, with vals, with non opts"},
	{test11_4, "--, with opts, with vals, with non opts"},
	{test11_5, "-- escaped, with opts, with vals, with non opts"},
	{test11_6, "-- escaped, with opts, with vals, with non opts"},
	{test12_0, "long opt with 1 char"},
	{test13_0, "hsearch"},
	TEST_NULL
};

int
main(int argc, char **argv)
{
	int i, cnt = 0, ret, total = 0;

	init();

	for(i = 0; tests[i].test; i++)
		total++;

	for(i = 0; tests[i].test; i++) {
		pretest(i);
		fprintf(stderr, "%s:%s... ", argv[0], tests[i].title);
		ret = tests[i].test();
		fprintf(stderr, "%s\n", ret ? "ok" : "FAIL");
		posttest(i);
		if (ret)
			cnt++;
	}

	deinit();

	printf("%s:TOTAL %d/%d\n", argv[0], cnt, total);
	return cnt != total;
}
