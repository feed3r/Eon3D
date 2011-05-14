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



/** \def log descriptor, opaque for the client. */
typedef struct CX_LogContext_ CX_LogContext; 

/** \enum the message levels */
typedef enum {
    CX_LOG_CRITICAL = 0, /**< this MUST be the first and it is
                              the most important. -- PANIC!       */
    CX_LOG_ERROR,        /**< you'll need to see this             */
    CX_LOG_WARNING,      /**< you'd better to see this            */
    CX_LOG_INFO,         /**< informative messages (for tuning)   */
    CX_LOG_DEBUG,        /**< debug messages (for devs)           */
    CX_LOG_MARK,         /**< verbatim, don't add anything, ever. */
    CX_LOG_LAST          /**< this MUST be the last -- 
                              and should'nt be used               */
} CX_LogLevel;


/** \var typedef CX_LogHandler
    \brief logging callback function.

    This callback is invoked by the eon3d runtime whenever is needed
    to log a message.

    eon3d provides a default callback to log to the  stderr.

    \param user_data a pointer given at the callback registration
           time. Fully opaque for eon3d.
    \param tag string identifying the eon3d module/subsystem.
    \param level the severity of the message.
    \param fmt printf-like format string for the message.
    \param args va_list of the arguments to complete the format string.
    \return 0 on success, <0 otherwise.

    \see CX_LogLevel
*/
typedef int (*CX_LogHandler)(void *user_data,
                             CX_LogLevel level, const char *tag,
                             const char *fmt, va_list args);



/** \fn open a log descriptor which discardes everything is send to it. */
/** Open a log descriptor. Every message sent to the returned log
    descriptor is silently and succesfully discarded.
    Any operation subsequent the succesfull open will always succeed.

    \return a pointer to the allocated descriptor, NULL on error.
    \see CX_log_close
*/
CX_LogContext *CX_log_open_null();

/** \fn open a log descriptor which sends everything to the given FILE. */
/** Attach a log descriptor to a given file. The file must be already open
    for write. The only operations performed on the file will be
    write() and flush().

    \param max_level the maximum level of severity of the messages
           sent. Messages tagged with a severity higher than
           this parameter will be silently discarded.
    \param sink the file to be used as log destination.
           MUST BE not NULL.
    \return a pointer to the allocated descriptor, NULL on error.
    \see CX_log_close
*/
CX_LogContext *CX_log_open_file(CX_LogLevel max_level, FILE *sink);

/** \fn open a log descriptor with colored output. */
/** Like CX_log_open_file, but using colored output using the
    VT100 ASCII sequences and with automatic fallback to stderr.
    Tested only on POSIX.

    \param max_level the maximum level of severity of the messages
           sent. Messages tagged with a severity higher than
           this parameter will be silently discarded.
    \param sink the file to be used as log destination.
           If NULL, falls back to stderr automatically.
    \return a pointer to the allocated descriptor, NULL on error.
    \see CX_log_open_file 
    \see CX_log_close
*/
CX_LogContext *CX_log_open_console(CX_LogLevel max_level, FILE *sink);

/**\fn open a custom log descriptor. */
/** A custom log descriptor uses a client-provided log callback
    to actually deliver the log message.
    Used that way, the logkit does NOT do the message formatting.

    \param max_level the maximum level of severity of the messages
          sent. Messages tagged with a severity higher than
          this parameter will be silently discarded. Thus, the callback
          is not invoked.
    \param log_handler the logging callback. Will be invoked each
           time the message is sent to the logger.
    \param user_data opaque data to be supplied to the callback at
           each call. WILL NOT be free()d at close.
    \return a pointer to the allocated descriptor, NULL on error.
    \see CX_log_close
*/
CX_LogContext *CX_log_open_custom(CX_LogLevel max_level,
                                  CX_LogHandler log_handler,
                                  void *user_data);

/** \fn close a log descriptor allocated using any CX_log_open_* */
/** Close a log descriptor and frees any acquired resource.
    Do not alter any attached or given resource (e.g. the FILE sink)

    \param ctx the log descriptor to close.
    \return 0 if succesfull, <0 on error.
*/
int CX_log_close(CX_LogContext *ctx);

/** \fn log a message using a given log descriptor */
/** log a message.

    \param ctx the log descriptor to use for logging.
    \param level the severity level of the message.
    \param tag log subsystem identifier.
    \param fmt printf(3) like format string. Format parameters follow.
    \return 0 if succesfull, <0 on error, >0 if the message was truncated.
    \see CX_LogLevel
*/
int CX_log_trace(CX_LogContext *ctx,
                 CX_LogLevel level, const char *tag, const char *fmt, ...);

/** \fn log a variable argumentsmessage using a given log descriptor */
/** log a message, given the variable arguments

    \param ctx the log descriptor to use for logging.
    \param level the severity level of the message.
    \param tag log subsystem identifier.
    \param fmt vprintf(3) like format string. Format parameters follow.
    \param args variable arguments, like vprintf.
    \return 0 if succesfull, <0 on error, >0 if the message was truncated.
    \see CX_LogLevel
    \see CX_log_trace
*/
int CX_log_trace_va(CX_LogContext *ctx,
                    CX_LogLevel level, const char *tag, const char *fmt,
                    va_list args);

/** \fn flush any buffered log to the destination */
/** 
    \param ctx the log descriptor to use for logging.
    \return 0 if succesfull, <0 on error.
*/
int CX_log_flush(CX_LogContext *cxt);


#endif  /* CX_LOGKIT_H */

