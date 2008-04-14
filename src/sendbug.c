/*	$OpenBSD: sendbug.c,v 1.49 2007/05/11 02:07:47 ray Exp $	*/

/*
 * Written by Ray Lai <ray@cyth.net>.
 * Public domain.
 */
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atomicio.h"
#include "lib.h"

#ifndef _PATH_DMESG
#define _PATH_DMESG "/var/log/dmesg"
#endif
#ifndef _PATH_SENDMAIL
#define _PATH_SENDMAIL "/usr/sbin/sendmail"
#endif
#define _DEFAULT_CONFIG "/etc/sendbug/sendbug.conf"

int	checkfile(const char *);
void	attatch(FILE *);
int	editit(const char *);
void	init(void);
static void	read_config(const char *);
int	matchline(const char *, const char *, size_t);
int	prompt(void);
int	send_file(const char *, int);
int	sendmail(const char *);
void	template(FILE *);

const char *categories = "acf aports base doc misc hosting";
char *version = "4.2";
const char *config_file;

struct passwd *pw;
//char os[BUFSIZ], rel[BUFSIZ], 
char release[BUFSIZ]; 
//char details[BUFSIZ];
struct utsname uts;
char *fullname, *tmppath, *pr_form, *mailfrom, *mailto;
const char *attatchment = NULL;
int wantcleanup;

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-d | -a file] [-LPV] [-c config]\n", __progname);
	exit(1);
}

void
cleanup()
{
	if (wantcleanup && tmppath && unlink(tmppath) == -1)
		warn("unlink");
}

static void
fcopy(FILE *srcfp, FILE *dstfp)
{
	char buf[BUFSIZ];
	size_t len;
	while (!feof(srcfp)) {
		len = fread(buf, 1, sizeof buf, srcfp);
		if (len == 0)
			break;
		if (fwrite(buf, 1, len, dstfp) != len)
			break;
	}
}

int
main(int argc, char *argv[])
{
	int ch, c, fd, ret = 1;
	const char *tmpdir;
	struct stat sb;
	time_t mtime;
	FILE *fp;

	if (access(_PATH_SENDMAIL, R_OK | X_OK) == -1) {
		warn("sendmail");
		fprintf(stderr, "Please run 'setup-sendbug' to configure sendbug\n");
		return ret;
	}

	config_file = NULL;
	while ((ch = getopt(argc, argv, "a:c:dLPV")) != -1)
		switch (ch) {
		case 'a':
			attatchment = optarg;
			break;
		case 'c':
			config_file = optarg;
			break;
		case 'd':
			attatchment = _PATH_DMESG;
			break;
		case 'L':
			printf("Known categories:\n");
			printf("%s\n\n", categories);
			exit(0);
		case 'P':
			init();
			template(stdout);
			exit(0);
		case 'V':
			printf("%s\n", version);
			exit(0);
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	if ((tmpdir = getenv("TMPDIR")) == NULL || tmpdir[0] == '\0')
		tmpdir = _PATH_TMP;
	if (asprintf(&tmppath, "%s%sp.XXXXXXXXXX", tmpdir,
	    tmpdir[strlen(tmpdir) - 1] == '/' ? "" : "/") == -1)
		err(1, "asprintf");
	if ((fd = mkstemp(tmppath)) == -1)
		err(1, "mkstemp");
	wantcleanup = 1;
	atexit(cleanup);
	if ((fp = fdopen(fd, "w+")) == NULL)
		err(1, "fdopen");

	init();

	if (pr_form == NULL)
		pr_form = getenv("PR_FORM");
	if (pr_form) {
		char buf[BUFSIZ];
		size_t len;
		FILE *frfp;

		frfp = fopen(pr_form, "r");
		if (frfp == NULL) {
			warn("can't seem to read your template file "
			    "(`%s'), ignoring PR_FORM", pr_form);
			template(fp);
		} else {
			fcopy(frfp, fp);
			fclose(frfp);
		}
	} else
		template(fp);

	if (!isatty(0)) {
		fcopy(stdin, fp);
		if (freopen(_PATH_TTY, "r", stdin) == NULL)
			err(1, _PATH_TTY);
	}

	if (fflush(fp) == EOF || fstat(fd, &sb) == -1 || fclose(fp) == EOF)
		err(1, "error creating template");
	mtime = sb.st_mtime;

 edit:
	if (editit(tmppath) == -1)
		err(1, "error running editor");

	if (stat(tmppath, &sb) == -1)
		err(1, "stat");
	if (mtime == sb.st_mtime)
		errx(1, "report unchanged, nothing sent");

 prompt:
	if (!checkfile(tmppath))
		fprintf(stderr, "fields are blank, must be filled in\n");
	c = prompt();
	switch (c) {
	case 'a':
	case EOF:
		wantcleanup = 0;
		errx(1, "unsent report in %s", tmppath);
	case 'e':
		goto edit;
	case 's':
		if (sendmail(tmppath) == -1)
			goto quit;
		break;
	default:
		goto prompt;
	}

	ret = 0;
quit:
	return (ret);
}

void
attatch(FILE *fp)
{
	char buf[BUFSIZ];
	FILE *dfp;
	off_t offset = -1;
	size_t len;

	dfp = fopen(attatchment, "r");
	if (dfp == NULL) {
		warn("can't read %s", attatchment);
		return;
	}

	fprintf(fp, "\n"
	    "<%s is attached.>\n", attatchment);

	fcopy(dfp, fp);
	fclose(dfp);
}

/*
 * Execute an editor on the specified pathname, which is interpreted
 * from the shell.  This means flags may be included.
 *
 * Returns -1 on error, or the exit value on success.
 */
int
editit(const char *pathname)
{
	char *argp[] = {"sh", "-c", NULL, NULL}, *ed, *p;
	sig_t sighup, sigint, sigquit;
	pid_t pid;
	int saved_errno, st;

	ed = getenv("VISUAL");
	if (ed == NULL || ed[0] == '\0')
		ed = getenv("EDITOR");
	if (ed == NULL || ed[0] == '\0')
		ed = _PATH_VI;
	if (asprintf(&p, "%s %s", ed, pathname) == -1)
		return (-1);
	argp[2] = p;

	sighup = signal(SIGHUP, SIG_IGN);
	sigint = signal(SIGINT, SIG_IGN);
	sigquit = signal(SIGQUIT, SIG_IGN);
	if ((pid = fork()) == -1)
		goto fail;
	if (pid == 0) {
		execv(_PATH_BSHELL, argp);
		_exit(127);
	}
	while (waitpid(pid, &st, 0) == -1)
		if (errno != EINTR)
			goto fail;
	free(p);
	(void)signal(SIGHUP, sighup);
	(void)signal(SIGINT, sigint);
	(void)signal(SIGQUIT, sigquit);
	if (!WIFEXITED(st)) {
		errno = EINTR;
		return (-1);
	}
	return (WEXITSTATUS(st));

 fail:
	saved_errno = errno;
	(void)signal(SIGHUP, sighup);
	(void)signal(SIGINT, sigint);
	(void)signal(SIGQUIT, sigquit);
	free(p);
	errno = saved_errno;
	return (-1);
}

int
prompt(void)
{
	int ret, c;

	__fpurge(stdin);
	fprintf(stderr, "a)bort, e)dit, or s)end: ");
	fflush(stderr);
	ret = getchar();
	if (ret == EOF || ret == '\n')
		return (ret);
	do {
		c = getchar();
	} while (c != EOF && c != '\n');

	return (ret);
}

int
sendmail(const char *pathname)
{
	int filedes[2];

	if (pipe(filedes) == -1) {
		warn("pipe: unsent report in %s", pathname);
		return (-1);
	}
	switch (fork()) {
	case -1:
		warn("fork error: unsent report in %s",
		    pathname);
		return (-1);
	case 0:
		close(filedes[1]);
		if (dup2(filedes[0], STDIN_FILENO) == -1) {
			warn("dup2 error: unsent report in %s",
			    pathname);
			return (-1);
		}
		close(filedes[0]);
		execl("/usr/sbin/sendmail", "sendmail",
		    "-oi", "-t", (void *)NULL);
		warn("sendmail error: unsent report in %s",
		    pathname);
		return (-1);
	default:
		close(filedes[0]);
		/* Pipe into sendmail. */
		if (send_file(pathname, filedes[1]) == -1) {
			warn("send_file error: unsent report in %s",
			    pathname);
			return (-1);
		}
		close(filedes[1]);
		wait(NULL);
		break;
	}
	return (0);
}

static void readfile(const char *filename, char *buf, size_t size)
{
	int count;
	int fd = open(filename, O_RDONLY);
	buf[0] = '\0';
	if (fd < 0)
		return;
	count = read(fd, buf, size-1);
	if (count < 0)
		return;
	buf[count] = '\0';
	close(fd);
	return;
}

static void *xmalloc(size_t size)
{
	void *ptr;
	ptr = malloc(size);
	if (ptr == NULL)
		err(1, "malloc");
	return ptr;
}

static char *xstrdup(const char *str)
{
	char *p;
	p = strdup(str);
	if (p == NULL)
		err(1, "strdup");
	return p;
}

static void read_config(const char *filename)
{
	FILE *fh;
	char *line = xmalloc(4096);

	fh = fopen(filename, "r");
	if (fh == NULL) 
		err(1, filename);
	while (fgets(line, 4095, fh) != NULL) {
		char *p;
		/* strip comments and newline */
		p = strpbrk(line, "#\n");
		if (p) *p = '\0';

		/* find keyword */
		p = strchr(line, '=');
		if (p == NULL)
			continue;
		*p++ = '\0';
	
		/* pr_form, editor, mailfrom, */
		if (strcmp(line, "form") == 0) {
			pr_form = xstrdup(p);
		} else if (strcmp(line, "mailfrom") == 0) {
			mailfrom = xstrdup(p);
		} else if (strcmp(line, "mailto") == 0) {
			mailto = xstrdup(p);
		} else 
			warn("ignoring unknown keyword: %s", line);
	}
	fclose(fh);		
	free(line);
}

void
init(void)
{
	size_t amp, len, gecoslen, namelen;
	int sysname[2];
	char ch, *cp;

	if (config_file == NULL)
		config_file = getenv("SENDBUG_CONF");
	if (config_file == NULL || config_file[0] == '\0')
		config_file = _DEFAULT_CONFIG;
	read_config(config_file);

	if ((pw = getpwuid(getuid())) == NULL)
		err(1, "getpwuid");
	if (fullname == NULL) {
		namelen = strlen(pw->pw_name);

		/* Count number of '&'. */
		for (amp = 0, cp = pw->pw_gecos; *cp && *cp != ','; ++cp)
			if (*cp == '&')
				++amp;

		/* Truncate gecos to full name. */
		gecoslen = cp - pw->pw_gecos;
		pw->pw_gecos[gecoslen] = '\0';

		/* Expanded str = orig str - '&' chars + concatenated logins. */
		len = gecoslen - amp + (amp * namelen) + 1;
		fullname = xmalloc(len);

		/* Upper case first char of login. */
		ch = pw->pw_name[0];
		pw->pw_name[0] = toupper((unsigned char)pw->pw_name[0]);

		cp = pw->pw_gecos;
		fullname[0] = '\0';
		while (cp != NULL) {
			char *token;

			token = strsep(&cp, "&");
			if (token != pw->pw_gecos &&
			    strlcat(fullname, pw->pw_name, len) >= len)
				errx(1, "truncated string");
			if (strlcat(fullname, token, len) >= len)
				errx(1, "truncated string");
		}

		/* Restore case of first char of login. */
		pw->pw_name[0] = ch;
	}
	
	if (mailfrom == NULL)
		mailfrom = getenv("PR_MAILFROM");
	if (mailfrom == NULL)
		mailfrom = pw->pw_name;

	if (mailto == NULL)
		mailto = getenv("PR_MAILTO");
	if (mailto == NULL)
		mailto = "bugs@alpinelinux.org";

	uname(&uts);

	readfile("/etc/alpine-release", release, sizeof(release)-1);
}

int
send_file(const char *file, int dst)
{
	int blank = 0;
	size_t len;
	char *buf;
	FILE *fp;

	if ((fp = fopen(file, "r")) == NULL)
		return (-1);
	while ((buf = fgetln(fp, &len))) {
		/* Skip lines starting with "SENDBUG". */
		if (len >= sizeof("SENDBUG") - 1 &&
		    memcmp(buf, "SENDBUG", sizeof("SENDBUG") - 1) == 0)
			continue;
		if (len == 1 && buf[0] == '\n')
			blank = 1;
		/* Skip comments, but only if we encountered a blank line. */
		while (len) {
			char *sp = NULL, *ep = NULL;
			size_t copylen;

			if (blank && (sp = memchr(buf, '<', len)) != NULL)
				ep = memchr(sp, '>', len - (sp - buf + 1));
			/* Length of string before comment. */
			if (ep)
				copylen = sp - buf;
			else
				copylen = len;
			if (atomicio(vwrite, dst, buf, copylen) != copylen) {
				int saved_errno = errno;

				fclose(fp);
				errno = saved_errno;
				return (-1);
			}
			if (!ep)
				break;
			/* Skip comment. */
			len -= ep - buf + 1;
			buf = ep + 1;
		}
	}
	fclose(fp);
	return (0);
}

/*
 * Does line start with `s' and end with non-comment and non-whitespace?
 * Note: Does not treat `line' as a C string.
 */
int
matchline(const char *s, const char *line, size_t linelen)
{
	size_t slen;
	int comment;

	slen = strlen(s);
	/* Is line shorter than string? */
	if (linelen <= slen)
		return (0);
	/* Does line start with string? */
	if (memcmp(line, s, slen) != 0)
		return (0);
	/* Does line contain anything but comments and whitespace? */
	line += slen;
	linelen -= slen;
	comment = 0;
	while (linelen) {
		if (comment) {
			if (*line == '>')
				comment = 0;
		} else if (*line == '<')
			comment = 1;
		else if (!isspace((unsigned char)*line))
			return (1);
		++line;
		--linelen;
	}
	return (0);
}

/*
 * Are all required fields filled out?
 */
int
checkfile(const char *pathname)
{
	FILE *fp;
	size_t len;
	int category, class, priority, release, severity, synopsis;
	char *buf;

	if ((fp = fopen(pathname, "r")) == NULL) {
		warn("%s", pathname);
		return (0);
	}
	category = class = priority = release = severity = synopsis = 0;
	while ((buf = fgetln(fp, &len))) {
		if (matchline(">Category:", buf, len))
			category = 1;
		else if (matchline(">Class:", buf, len))
			class = 1;
		else if (matchline(">Priority:", buf, len))
			priority = 1;
		else if (matchline(">Release:", buf, len))
			release = 1;
		else if (matchline(">Severity:", buf, len))
			severity = 1;
		else if (matchline(">Synopsis:", buf, len))
			synopsis = 1;
	}
	fclose(fp);
	return (category && class && priority && release && severity &&
	    synopsis);
}

void
template(FILE *fp)
{
	fprintf(fp, "SENDBUG: -*- sendbug -*-\n");
	fprintf(fp, "SENDBUG: Lines starting with `SENDBUG' will"
	    " be removed automatically, as\n");
	fprintf(fp, "SENDBUG: will all comments (text enclosed in `<' and `>').\n");
	fprintf(fp, "SENDBUG:\n");
	fprintf(fp, "To: %s\n", mailto);
	fprintf(fp, "Subject: \n");
	fprintf(fp, "From: %s\n", mailfrom);
	fprintf(fp, "Cc: %s\n", mailfrom);
	fprintf(fp, "Reply-To: %s\n", mailfrom);
	fprintf(fp, "X-sendbug-version: %s\n", version);
	fprintf(fp, "\n");
	fprintf(fp, "\n");
	fprintf(fp, ">Submitter-Id:\tcurrent-users\n");
	fprintf(fp, ">Originator:\t%s\n", fullname);
	fprintf(fp, ">Organization:\n");
	fprintf(fp, ">Synopsis:\t<synopsis of the problem (one line)>\n");
	fprintf(fp, ">Severity:\t"
	    "<[ non-critical | serious | critical ] (one line)>\n");
	fprintf(fp, ">Priority:\t<[ low | medium | high ] (one line)>\n");
	fprintf(fp, ">Category:\t"
	    "<[ acf | aports | base | doc | misc | hosting ] (one line)>\n");
	fprintf(fp, ">Class:\t\t"
	    "<[ sw-bug | doc-bug | change-request | support ] (one line)>\n");
	fprintf(fp, ">Release:\t%s\n",release);
	fprintf(fp, ">Environment:\n");
	fprintf(fp, "\t<machine, os, target, libraries (multiple lines)>\n");
	fprintf(fp, "\tSystem      : %s\n", uts.sysname);
	fprintf(fp, "\tVersion     : %s\n", uts.version);
	fprintf(fp, "\tMachine     : %s\n", uts.machine);
	fprintf(fp, "\tRelease     : %s\n", uts.release);
	fprintf(fp, ">Description:\n");
	fprintf(fp, "\t<precise description of the problem (multiple lines)>\n");
	fprintf(fp, ">How-To-Repeat:\n");
	fprintf(fp, "\t<code/input/activities to reproduce the problem"
	    " (multiple lines)>\n");
	fprintf(fp, ">Fix:\n");
	fprintf(fp, "\t<how to correct or work around the problem,"
	    " if known (multiple lines)>\n");

	if (attatchment)
		attatch(fp);
}
