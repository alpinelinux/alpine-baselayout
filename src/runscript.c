#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAXARGS 32

int main(int argc, char *argv[]) {
	char *av[MAXARGS];
	int i;
	char *runscriptsh;

	if (!(runscriptsh = getenv("RUNSCRIPT")))
		runscriptsh = "/sbin/runscript-alpine.sh";
	for (i = 0; i < argc && i < MAXARGS ; i++) {
		av[i] = argv[i];
	}
	av[i] = NULL;
			
	if (execv(runscriptsh, av) < 0) {
		perror("execv");
		return -1;
	}
	return 0;
}
