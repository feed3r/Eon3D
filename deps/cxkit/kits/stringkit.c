/* 
 * String utilities kit - utility functions to work with C strings.
 * (C) 2010-2013 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file stringkit.h
    \brief the string utility kit implementation.
*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <errno.h>

#include "CX_kit.h"
#include "stringkit.h"
#include "memorykit.h"



char *CX__tstrndup(const char *file, int line, const char *s, size_t n)
{
    char *pc = NULL;

    if (s != NULL) {
        pc = CX__tmalloc(file, line, n + 1);
        if (pc != NULL) {
            memcpy(pc, s, n);
            pc[n] = '\0';
        }
    }
    return pc;
}

void CX_strstrip(char *s) 
{
    size_t len = 0;
    char *start = s;

    if (s == NULL) {
        return;
    }
    
    while ((*start != 0) && isspace(*start)) {
        start++;
    }
    
    memmove(s, start, strlen(start) + 1);

    len =  strlen(s);
    if (len > 0) {
        start = &s[len - 1];
        while ((start != s) && isspace(*start)) {
            *start = '\0';
            start--;
        }
    }
    return;
}

char **CX_strsplit(const char *str, char sep, size_t *pieces_num)
{
    const char *begin = str, *end = NULL;
    char **pieces = NULL, *pc = NULL;
    size_t i = 0, n = 2;
    int failed = CX_FALSE;

    if (!str || !strlen(str)) {
        return NULL;
    }

    while (begin != NULL) {
        begin = strchr(begin, sep);
        if (begin != NULL) {
            begin++;
            n++;
        }
    }

    pieces = CX_malloc(n * sizeof(char*));
    if (!pieces) {
        return NULL;
    }

    begin = str;
    while (begin != NULL) {
        size_t len;

        end = strchr(begin, sep);
        if (end != NULL) {
            len = (end - begin);
        } else {
            len = strlen(begin);
        }
        if (len > 0) {
            pc = CX_strndup(begin, len);
            if (pc == NULL) {
                failed = CX_TRUE;
                break;
            } else {
                pieces[i] = pc;
                i++;
            }
        }
        if (end != NULL) {
            begin = end + 1;
        } else {
            break;
        }
    }

    if (failed) {
        /* one or more copy of pieces failed */
        CX_free(pieces);
        pieces = NULL;
    } else { /* i == n - 1 -> all pieces copied */
        pieces[n - 1] = NULL; /* end marker */
        if (pieces_num != NULL) {
            *pieces_num = i;
        }
    }
    return pieces;
}

void CX_strfreev(char **pieces)
{
    if (pieces != NULL) {
        int i = 0;
        for (i = 0; pieces[i] != NULL; i++) {
            CX_free(pieces[i]);
        }
        CX_free(pieces);
    }
}


/*
 * Safer string functions from OpenBSD, because these are not in every
 * libc implementations.
 */

/*************************************************************************/
/* intentionally included the whole file                                 */
#ifndef CX_HAVE_STRLCPY
/*	$OpenBSD: strlcpy.c,v 1.9 2005/03/30 20:13:52 otto Exp $	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: strlcpy.c,v 1.9 2005/03/30 20:13:52 otto Exp $";
#endif /* LIBC_SCCS and not lint */
#endif /* 0 */

/* #include <sys/types.h> */
/* #include <string.h> */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

#endif  /* CX_HAVE_STRLCPY */
/*************************************************************************/
/* intentionally included the whole file                                 */
#ifndef CX_HAVE_STRLCAT
/*	$OpenBSD: strlcat.c,v 1.12 2005/03/30 20:13:52 otto Exp $	*/

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: strlcat.c,v 1.12 2005/03/30 20:13:52 otto Exp $";
#endif /* LIBC_SCCS and not lint */
#endif  /* 0 */

/* #include <sys/types.h> */
/* #include <string.h> */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}

#endif  /* CX_HAVE_STRLCAT */
/*************************************************************************/
/* frontends                                                             */

size_t CX_strlcpy(char *dst, const char *src, size_t siz)
{
    return strlcpy(dst, src, siz);
}

size_t CX_strlcat(char *dst, const char *src, size_t siz)
{
    return strlcat(dst, src, siz);
}


/* vim: et sw=4 ts=4: */

