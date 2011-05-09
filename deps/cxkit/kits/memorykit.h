/* 
 * Memory utilities kit - utility functions to track memory handling.
 * (C) 2010-2011 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file memorykit.h
    \brief the Memory handling kit interface.
*/

#ifndef CX_MEMORYKIT_H
#define CX_MEMORYKIT_H

#include "CX_config.h"

#include <stdlib.h>
#include <stdint.h>


/** \struct an opaque memory chunk */
typedef struct CX_memchunk_ CX_MemChunk;
struct CX_memchunk_ {
    void  *data; /**< base pointer */
    size_t size; /**< chunk size   */
};

/** \def malloc(3) replacement which traces allocation failures.
    \see the CX__t* corrispondent.
*/
#define CX_malloc(size)    CX__tmalloc(__FILE__, __LINE__, size)

/** \def malloc(3) replacement which returns a zero-filled
    chunk and traces allocation failures.
    \see the CX__t* corrispondent.
*/
#define CX_zalloc(size)    CX__tzalloc(__FILE__, __LINE__, size)

/** \def realloc(3) replacement which traces allocation failures.
    \see the CX__t* corrispondent.
*/
#define CX_realloc(ptr, size) CX__trealloc(__FILE__, __LINE__, ptr, size)

/** \def free(3) replacement of API completeness */
#define CX_free(ptr)       free(ptr)

/** \fn malloc(3) replacement which traces the allocation failures. */
/**
    \param file name of the source code file containing the call.
    \param line line of the source code file containing the call.
    \param size size in bytes of the memory block to allocate.
    \return a pointer to the allocated block, NULL on error.
*/
void *CX__tmalloc(const char *file, int line, size_t size);

/** \fn malloc(3) replacement which traces the allocation failures. */
/** Returns, if succesfull, a zero-filled memory block.

    \param file name of the source code file containing the call.
    \param line line of the source code file containing the call.
    \param size size in bytes of the memory block to allocate.
    \return a pointer to the allocated block, NULL on error.
*/
void *CX__tzalloc(const char *file, int line, size_t size);

/** \fn realloc(3) replacement which traces the allocation failures. */
/** 
    \param file name of the source code file containing the call.
    \param line line of the source code file containing the call.
    \param ptr pointer to reallocate.
    \param size new size in bytes of the memory block to reallocate.
    \return a newpointer to the reallocated block, NULL on error.
*/
void *CX__trealloc(const char *file, int line, void *ptr, size_t size);

/** \def malloc(3)-like which provides a page-aligned allocation and
    traces allocation failures.
    \see the CX__t* corrispondent.
*/
#define CX_paalloc(size) CX__tpaalloc(__FILE__, __LINE__, size)

/** \fn malloc(3) replacement which traces the allocation failures. */
/** Returns, if succesfull, a page-aligned memory block.
    Due to alignment constraints, the eventual allocation WILL be
    higher than the requested chunk. The padding area
    The caller MUST free the allocated block with CX_buffree.

    \param file name of the source code file containing the call.
    \param line line of the source code file containing the call.
    \param size size in bytes of the memory block to return.
    \return a pointer to the allocated block, NULL on error.
    \see CX_buffree
*/
void *CX__tpaalloc(const char *file, int line, size_t size);

/** \fn free(3) replacement for deallocating chunks obtained through
    CX__tpaalloc.*/
/** DO NOT use this function to release blocks obtained from other
    *allocs, or things will fail miserably.

    \param ptr pointer to release.
    \see CX__tpaalloc
*/
void CX_pafree(void *ptr);

#endif  /* CX_MEMORYKIT_H */

