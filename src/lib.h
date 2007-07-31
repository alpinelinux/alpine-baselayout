#ifndef RCS_LIB_H
#define RCS_LIB_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>

/* strlcpy and strlcat */
#ifndef HAVE_STRLCPY
size_t strlcpy(char *, const char *, size_t);
#endif

#ifndef HAVE_STRLCPY
size_t strlcat(char *, const char *, size_t);
#endif

#ifndef HAVE_STRTONUM
long long strtonum(const char *, long long, long long, const char **);
#endif

#ifndef HAVE_FGETLN
char *fgetln(FILE *, size_t *);
#endif

#endif /* RCS_LIB_H */
