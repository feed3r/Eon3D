/* 
 * Vector ARRAY kit - an array which automatically enlarges when needed.
 * (C) 2010-2013 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file arraykit.h
    \brief the Variable Array kit interface.
*/

#ifndef CX_ARRAYKIT_H
#define CX_ARRAYKIT_H

#include <stdint.h>

/** \enum Error codes for the varray functions. */
/** DO NOT rely on their values. ABI isn't guaranteed */
typedef enum {
    CX_VARRAY_OK       =  0, /**< success (no real error, thus)          */
    CX_VARRAY_EMPTY    = -1, /**< empty CX_VArray element (not critical) */
    CX_VARRAY_BAD_REF  = -2, /**< bad varray reference given             */
    CX_VARRAY_BAD_IDX  = -3, /**< bad varray index given                 */
    CX_VARRAY_NO_MEM   = -4  /**< out of memory                          */
} CX_VArrayError;

/** \def opaque for the client */
typedef struct CX_varray_ CX_VArray;

/** \fn Allocates a new CX_VArray. */
/**
  \param size initial number of elements.
  \param element_size size of each element of the CX_VArray.
         `0' could be used as shourcut for sizeof(pointer).
  \return a new CX_VArray reference on success, NULL on error
  \see CX_varray_del
*/
CX_VArray *CX_varray_new(int32_t size, int32_t element_size);

/** \fn Disposes a CX_VArray. */
/**
  \param va a CX_VArray reference.
  \return CX_VARRAY_OK on success, a CX_VArrayError otherwise.
  \see CX_varray_new
*/
int CX_varray_del(CX_VArray *va);

/** \fn Gets the length of a CX_VArray. */
/** The length of a CX_VArray is the number of the element present
    in the CX_VArray.
  \param va a CX_VArray reference.
  \return >= 0 the length of the CX_VArray, < 0 a CX_VArrayError otherwise.
*/
int32_t CX_varray_length(CX_VArray *va);

/** \fn Gets the size of a CX_VArray. */
/** The size of a CX_VArray is the amount of the memory occupied, including the
    length of the CX_VArray plus the spare space already reserved.
  \param va a CX_VArray reference.
  \return >= 0 the size of the CX_VArray, < 0 a CX_VArrayError otherwise.
  \see CX_varray_length
*/
int32_t CX_varray_size(CX_VArray *va);

/** \fn Gets the element size of a CX_VArray. */
/** Useful mostly for debug purposes since the caller is supposed to know
    the element size which it requested to the CX_VArray.
  \param va a CX_VArray reference.
  \return > 0 the size of the CX_VArray, <= 0 a CX_VArrayError otherwise.
  \see CX_varray_new
*/
int32_t CX_varray_element_size(CX_VArray *va);

/** \fn Inserts an element in a Varray in a given position. */
/** Always overwrites the old element.
    Automatically enforce the bound checking: you cannot access
    after the end or before the begin of the CX_VArray.
    Always copies the given element reference into the array.
  \param va a CX_VArray reference.
  \param position in the CX_VArray (like a plain C array) of the element.
  \param element const pointer of the new element.
  \return CX_VARRAY_OK on success, a CX_VArrayError otherwise.
  \see CX_varray_append
  \see CX_varray_add
*/
int CX_varray_insert(CX_VArray *va, int32_t position, const void *element);

/** \fn Inserts an element in a CX_VArray in the last position. */
/**
  \param va a CX_VArray reference.
  \param element const pointer of the new element.
  \return CX_VARRAY_OK on success, a CX_VArrayError otherwise.
  \see CX_varray_insert
  \see CX_varray_add
*/
int CX_varray_append(CX_VArray *va, const void *element);

/** \fn Removes an element from a CX_VArray given its position. */
/** This function also shrinks the varray by shifting to the left all
    the elements after the removed one.
  \param va a CX_VArray reference.
  \param position in the CX_VArray (like a plain C array) of the element.
  \return CX_VARRAY_OK on success, a CX_VArrayError otherwise.
  \see CX_varray_insert
  \see CX_varray_append
*/
int CX_varray_remove(CX_VArray *va, int32_t position);

/** \fn Gets an element from a CX_VArray given its position. */
/** This functions gets A COPY of a given element (by position)
    the caller must allocate enough storage for a copy of the
    element.
    Automatically enforce the bound checking: you cannot access
    after the end or before the begin of the CX_VArray.
  \param va a CX_VArray reference.
  \param position in the CX_VArray (like a plain C array) of the element.
  \param element pointer to storage to hold the copy of the CX_VArray
         element.
  \return CX_VARRAY_OK on success, a CX_VArrayError otherwise.
*/
int CX_varray_get(CX_VArray *va, int32_t position, void *element);

/** \fn Gets a reference to element from a CX_VArray given its position. */
/** This functions gets A POINTER of a given element (by position)
    the caller must take extra care in handling the pointer since
    it points to the CX_VArray intenal structure.
    As rule of thumb, any read is safe, while writes should be carefully
    evaluated.
    Automatically enforce the bound checking: you cannot access
    after the end or before the begin of the CX_VArray.
  \param va a CX_VArray reference.
  \param position in the CX_VArray (like a plain C array) of the element.
  \return a pointer to the element on success, NULL otherwise.
          Notice you'll get NULL if you try to access a Removed element
  \see CX_varray_remove
*/
void *CX_varray_get_ref(CX_VArray *va, int32_t position);

/**\fn Resets the varray to a pristine state without deallocating */
/** Use this function when you want to recycle a VArray and you
    want to avoid a new round of grow()s, given that the new size
    of the varray will be something closer to the current one.
  \param va a CX_VArray reference.
*/
int CX_varray_reset(CX_VArray *va);


#endif /* CX_ARRAYKIT_H */

