/* 
 * Logging kit - basic pluggable logging utilities.
 * (C) 2011 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file logkit.h
    \brief the LOGging kit interface.
*/

#ifndef CX_LOGKIT_H
#define CX_LOGKIT_H

#include "CX_config.h"

#include <stdarg.h>
#include <stdio.h>


/** \enum the message severity/type */
typedef enum {
    CX_LOG_ERR   = 0, /**< critical error condition           */
    CX_LOG_WARN,      /**< non-critical error condition       */
    CX_LOG_INFO,      /**< informative highlighted message    */
    CX_LOG_DEBUG,     /**< debug message                      */
    CX_LOG_MARK       /**< verbatim, don't add anything, ever */
} CX_LogType;


/** \def opaque for the client */
typedef struct CX_LogContext_ CX_LogContext; 



CX_LogContext *CX_log_open_null(CX_VerboseLevel verbose);

CX_LogContext *CX_log_open_file(CX_VerboseLevel verbose, FILE *sink);

CX_LogContext *CX_log_open_console(CX_VerboseLevel verbose, FILE *sink);

int CX_log_close(CX_LogContext *ctx);

int CX_log_trace(CX_LogContext *ctx,
                 CX_LogType type, const char *tag, const char *fmt, ...);


#endif  /* CX_LOGKIT_H */

