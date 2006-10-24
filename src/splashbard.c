/*

Splash daemon 

*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

char unfilled[64] = " ";
char filled[64] = "=";
char leftborder[64] = "[";
char rightborder[64] = "]";
char animation[64] = "\\|/-";
char message[128] = "SplashD";

int width = 78;
int delay = 50000;
int steps = 10;

char sigusr1 = 0;
char sigusr2 = 0;
char sigterm = 0;

const char options[] = "a:d:f:l:m:r:u:"; 

void usage(int exitcode) {
	fprintf(stderr, "Usage: splashd [-adflru] [OPTION] STEPS\n");
	exit(exitcode);
}

int parse_args(int argc, char *argv[]) {
	int c;
	while ((c = getopt(argc, argv, options)) > 0) {
		switch(c) {

		case 'a': {
			strncpy(animation, optarg, sizeof(animation));
			break;
		}

		case 'd': {
			delay = atoi(optarg);
			break;
		}

		case 'f': {
			strncpy(filled, optarg, sizeof(filled));
			break;
		}

		case 'l': {
			strncpy(leftborder, optarg, sizeof(leftborder));
			break;
		}

		case 'm': {
			strncpy(message, optarg, sizeof(message));
			break;
		}

		case 'r': {
			strncpy(rightborder, optarg, sizeof(rightborder));
			break;
		}

		case 'u': {
			strncpy(unfilled, optarg, sizeof(unfilled));
			break;
		}

		case '?':
		default: {
			usage(1);
			break;
		}

		}
    
	}
	if (optind != argc-1) usage(1);
	return atoi(argv[optind]);
}


void clear_screen(void) {
	int i;
	for (i=0; i<50; i++) {
		printf("\f");
	}
	printf("\n");
}

void center(char *s) {
	int i;
	printf("\n");
	for (i=0; i < (80-strlen(s)) /2; i++) putchar(' ');
	printf("%s", s);
}

void draw_bar(int steps, char *msg) {
	int i=0;
	char bar[256];
	char *p = bar;
	int size=sizeof(bar);

	if (steps >= 78) steps=78;

	for (i=0; i < 20; i++) printf("\n");
	p += snprintf(p, (size =- strlen(leftborder)), "%s", leftborder);

	for (i=0; i < steps; i++) {
			 p += snprintf(p, (size =-strlen(unfilled)), "%s", unfilled);
	}
	p += snprintf(p, (size =- strlen(rightborder)), "%s", rightborder);

	center(msg);
	printf("\n");
	center(bar);
	for (i=0; i<steps+1; i++) putchar('\b');
	fflush(stdout);
	
}

void sig_handler(int signo) {
	signal(signo, sig_handler);
	switch(signo) {
			
		case SIGUSR1: {
			sigusr1 = 1;
			break;
		}

		case SIGUSR2: {
			sigusr2 = 1;
			break;
		}

  		case SIGTERM: {
			sigterm = 1;
			break;
		}

		default:
			exit(0);
	}
}

void fill_barstr(char *buf, size_t totsize,
				int current, int steps, char curs) {
	int i = 0;
	int lbsize = strlen(leftborder);
	int rbsize = strlen(rightborder);
	int barsize = totsize - lbsize - rbsize;
	int fillcount = current * barsize / steps;
	int fillsize = strlen(filled) * fillcount;
	char *bar = buf + lbsize;
	
	if (totsize < lbsize + rbsize + 1) return;
	
	strncpy(buf, leftborder, totsize);
	memset(bar, unfilled[0], barsize);
	memset(bar, filled[0], fillcount);
	strncpy(bar + barsize, rightborder, totsize - rbsize);
	if ( current < steps) bar[fillcount] = curs;
}
								

void run_animation(int steps) {
	int current = 0;
	int i=0;
	char buf[256];
	while (current < steps && sigterm == 0) {
		fill_barstr(buf, width, current, steps, animation[i]);
		i = (i + 1) % strlen(animation);
		printf("\r%s", buf);
		usleep(delay);
		if (sigusr1) {
			sigusr1 = 0;
			current++;
		}
		
		fflush(stdout);
	}
	fill_barstr(buf, width, steps, steps, animation[i]);
	printf("\r%s", buf);
	fflush(stdout);
}


int main(int argc, char *argv[]) {
	int steps;

	steps = parse_args(argc, argv);

	/* clear screen */
//	clear_screen();

	/* fork */
	if (fork()) {
		exit(0);
	}

	/* attatch to signal handler */
	signal(SIGUSR1, sig_handler);
	signal(SIGTERM, sig_handler);

	run_animation(steps);
	printf("\n\n");
	return 0;
}
