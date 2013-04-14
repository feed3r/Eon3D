/* 
 * Vector ARRAY kit - an array which automatically enlarges when needed.
 * (C) 2010-2013 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arraykit.h"


struct CX_varray_ {
    void    *data;
    int32_t size;
    int32_t length;
    int32_t element_size;
};

/* VA_* / va_* for internal usage/private identifiers */

enum {
    VA_ELEMENT_SIZE_MIN = 1,
};

#define VA_CHECK_REF(VA) do { \
    if (!(VA)) { \
        return CX_VARRAY_BAD_REF; \
    } \
} while (0)

/* since we permit negative indexes, we have extra need for
   sanity checking */
static int va_adjust_position(CX_VArray *va, int32_t position)
{
    if (position < 0) {
        position = va->length - position;
    }
    if (position < 0) {
        position = 0;
    }
    if (position > va->length) {
        position = va->length;
    }
    return position;
}

static int va_grow(CX_VArray *va)
{
    int err = 0;

    void *ndata = realloc(va->data, va->size * 2);
    if (ndata) {
        memset(va->data + va->size, 0, va->size);
    } else {
        ndata = calloc(va->size * 2, va->element_size);
        if (ndata) {
            memcpy(ndata, va->data, va->size);
            free(va->data);
        }
    }
    
    if (ndata) {
        va->data = ndata;
        va->size *= 2;
    } else {
        err = -1;
    }
    return err;
}

static int va_prepare_put(CX_VArray *va, int32_t position)
{
    int ret = 0;
    position = va_adjust_position(va, position);
    if (position >= va->size) {
        ret = va_grow(va);
    }
    return ret;
}

static int32_t va_adjust_element_size(int32_t element_size)
{
    if (element_size <= 0) {
        element_size = sizeof(void*);
    }
    if (element_size <= VA_ELEMENT_SIZE_MIN) {
        element_size = VA_ELEMENT_SIZE_MIN;
    }

    return element_size;
}

CX_VArray *CX_varray_new(int32_t size, int32_t element_size)
{
    element_size = va_adjust_element_size(element_size);
    CX_VArray *va = calloc(1, sizeof(struct CX_varray_));
    if (va) {
        va->data = calloc(size, element_size);
        if (!va->data) {
            free(va);
            va = NULL;
        } else {
            va->size = size;
            va->element_size = element_size;
        }
    }
    return va;
}


int CX_varray_del(CX_VArray *va)
{
    VA_CHECK_REF(va);

    if (va->data) {
        free(va->data);
        va->data = NULL;
    }
    free(va);
    va = NULL;

    return CX_VARRAY_OK;
}

int32_t CX_varray_length(CX_VArray *va)
{
    VA_CHECK_REF(va);
    return va->length;
}

int32_t CX_varray_size(CX_VArray *va)
{
    VA_CHECK_REF(va);
    return va->size;
}

int32_t CX_varray_element_size(CX_VArray *va)
{
    VA_CHECK_REF(va);
    return va->element_size;
}

static void *va_ptr(CX_VArray *va, int32_t position)
{
    void *p = (uint8_t *)va->data + (position * va->element_size);
    return p;
}

int CX_varray_insert(CX_VArray *va, int32_t position, const void *element)
{
    int err = -1;

    VA_CHECK_REF(va);

    err = va_prepare_put(va, position);
    if (err == 0) {
        memcpy(va_ptr(va, position), element, va->element_size);
        va->length++;
    }
    return err;
}

int CX_varray_append(CX_VArray *va, const void *element)
{
    VA_CHECK_REF(va);
    return CX_varray_insert(va, va->length, element);
}

int CX_varray_remove(CX_VArray *va, int32_t position)
{
    int32_t to_move = 0;
    void *ptr = NULL;
    int err = 0;
    
    VA_CHECK_REF(va);
    
    position = va_adjust_position(va, position);
    ptr = va_ptr(va, position);
    to_move = (va->length - position) * va->element_size;

    memmove(ptr, ptr + va->element_size, to_move);
    va->length--;

    return err;
}

int CX_varray_get(CX_VArray *va, int32_t position, void *element)
{
    VA_CHECK_REF(va);
        
    position = va_adjust_position(va, position);
    if (element) {
        void *ptr = va_ptr(va, position);
        memcpy(element, ptr, va->element_size);
    }
    return CX_VARRAY_OK;
}

void *CX_varray_get_ref(CX_VArray *va, int32_t position)
{
    void *ptr = NULL;
    if (va) {
        position = va_adjust_position(va, position);
        ptr = va_ptr(va, position);
    }
    return ptr;
}

int CX_varray_reset(CX_VArray *va)
{
    VA_CHECK_REF(va);
    va->length = 0;
    return 0;
}


/* vim: et sw=4 ts=4: */

