#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sysexits.h>

static const char *program = "readahead";

void usage(void) {
	printf("usage: %s [-hv] FILE [...]\n", program);
	exit(EX_USAGE);
}

int main( int argc, char *argv[]) {
	int c, verbose=0;

	/* parse options */
	while ( (c = getopt(argc, argv, "hv")) >= 0 ) {
		switch (c) {
		case 'v':	verbose++;
				break;
		case 'h':
		default:	usage();
				break;
		}	
	}
	
	/* check that at least one file is specified */
	if (optind == argc)
		usage();
	
	/* parse files */
	c = EX_OK;
	while (optind < argc) {
		struct stat st;
		FILE *f = fopen(argv[optind], "r");


		/* check that file exists */
		if (f == NULL) {
			perror(argv[optind]);
			c = EX_NOINPUT;
		} else {
			stat(argv[optind], &st);
			readahead( fileno(f), 0, (size_t)st.st_size );
			if (verbose) 
				printf("%s\n", argv[optind]);
			fclose(f);
		}
		optind++;
	}
	return (c);
}

