/* Usage: ./ex2 -abc d -e f --g=h --i j -k*/

#include <stdio.h>
#include "egetopt.h"

int
cb(const char *oname, const char *oval, void *priv)
{
	printf("%s=%s\n", oname, oval);
	return 0;
}

int
main(int argc, char **argv)
{
	int i;

	argc = egetopt_parse(argc, argv, cb, NULL);
	for (i = 0; i < argc; i++)
		printf("%c %s", i ? ',' : ' ', argv[i]);
	printf("\n");

	return 0;
}

