/* 
 * cxkit is yet another collection of utilities for the C language.
 * (C) 2010-2011 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file CX_kit.h
    \brief the main cxkit header.
*/

#ifndef CX_KIT_H
#define CX_KIT_H

/* CX_kit depends on the following */
#include <stdint.h>
#include <stdbool.h>

#define CX_MAX(a, b)		(((a) > (b)) ?(a) :(b))
#define CX_MIN(a, b)		(((a) < (b)) ?(a) :(b))
/* clamp x between a and b */
#define CX_CLAMP(x, a, b)	CX_MIN(CX_MAX((a), (x)), (b))


#endif /* CX_KIT_H */

