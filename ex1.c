/* Usage: ./ex1 -abc d -h -e f --g=h --version --i j -v */

#include <stdio.h>
#include "egetopt.h"

int
cb(const struct egetopt_opt *odef, const char *oval, void *priv)
{
	if (odef == NULL) {
		printf("Unknown option: %s\n", oval);
		return 0;
	}
	switch (odef->val) {
	case 'h':
		printf("request -h");
		break;
	case 'v':
		printf("request -v");
		break;
	}
	printf("(%s=%s)\n", odef->name, oval);
	return 0;
}

int
main(int argc, char **argv)
{
	int i;

	struct egetopt_opt opts[] = {{"-h", 'h'}, {"--version", 'v'}, {"-v", 'v'}, {NULL, 0}};

	argc = egetopt_parse_hsearch(argc, argv, opts, cb, NULL);

	printf("non option arguments: ");
	for (i = 0; i < argc; i++)
		printf("%c %s", i ? ',' : ' ', argv[i]);
	printf("\n");

	return 0;
}

