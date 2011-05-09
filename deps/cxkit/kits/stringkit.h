/* 
 * String utilities kit - utility functions to work with C strings.
 * (C) 2010-2011 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file stringkit.h
    \brief the string utility kit interface.
*/

#ifndef CX_STRINGKIT_H
#define CX_STRINGKIT_H

#include "CX_config.h"

#include <string.h>
#include <stdarg.h>


/** \fn split a string into tokens using a given separator character. */
/** The split string is returned in a form of NULL-terminated array.
    This array shall be released using CX_strfreev.

    \param str the string to split.
    \param sep the separator CHARACER. The string is cut when `sep' is found.
    \param pieces_num if not NULL, will hold the number of pieces cut.
    \return a NULL-terminated array of split pieces if successfull,
            or NULL on error.
    \see CX_strfreev.
 */
char **CX_strsplit(const char *str, char sep, size_t *pieces_num);

/** \fn release an array returned by CX_strsplit. */
/**
    \param pieces the array of strings to release.
*/
void CX_strfreev(char **pieces);

/** \fn removes IN PLACE the heading and trailing whitespaces from
    a given string. */
/** It is safe to supply a NULL string.
    \param s the string to strip.
*/
void CX_strstrip(char *s);

/** \def strdup(3) replacement which traces allocation failures.
    \see the CX__t* corrispondent.
*/
#define CX_strdup(s) \
            CX__tstrndup(__FILE__, __LINE__, s, strlen(s))

/** \def strndup(3) replacement which traces allocation failures.
    \see the CX__t* corrispondent.
*/
#define CX_strndup(s, n) \
            CX__tstrndup(__FILE__, __LINE__, s, n)

/** \fn strndup(3) replacement which traces the allocation failures. */
/** the returned pointer shall be released using CX_free.
    \param file name of the source code file containing the call.
    \param line line of the source code file containing the call.
    \param size size in bytes of the memory block to allocate.
    \param s null-terminated string to copy.
    \param n copy at most 'n' characters of original string.
    \return a pointer to the allocated block, NULL on error.
    \see CX_free
*/
char *CX__tstrndup(const char *file, int line, const char *s, size_t n);

/** \fn strlcpy(3) replacement for system without it.*/
size_t CX_strlcpy(char *dst, const char *src, size_t siz);

/** \fn strlcat(3) replacement for system without it.*/
size_t CX_strlcat(char *dst, const char *src, size_t siz);


#endif  /* CX_STRINGKIT_H */

