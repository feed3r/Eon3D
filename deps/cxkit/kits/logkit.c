/* 
 * Logging kit - basic pluggable logging utilities.
 * (C) 2011 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file logkit.h
    \brief the LOGging kit implementation.
*/

#include "CX_kit.h"
#include "memorykit.h"
#include "logkit.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>


struct CX_LogContext_ {
    void *priv;

    CX_LogLevel max_level;

    CX_LogHandler trace;
    int (*close)(CX_LogContext *ctx);
    int (*flush)(CX_LogContext *ctx);
};


/* colors macros */
#define COL(x)              "\033[" #x ";1m"
#define COL_RED             COL(31)
#define COL_GREEN           COL(32)
#define COL_YELLOW          COL(33)
#define COL_BLUE            COL(34)
#define COL_WHITE           COL(37)
#define COL_GRAY            "\033[0m"

enum {
    CX_LOG_BUF_SIZE     = 1024,
    CX_LOG_TEMPLATE_LEN = 32 /* upper bound, really */
};

static const char *log_template(CX_LogLevel level)
{
    /* WARNING: we MUST keep in sync templates order with CX_LOG* macros */
    static const char *CX_log_templates[] = {
        COL_RED"CRI [%s]: %s"COL_GRAY"\n",    /* CX_LOG_CRITICAL */
        COL_RED"ERR [%s]"COL_GRAY": %s\n",    /* CX_LOG_ERROROR    */
        COL_YELLOW"WRN [%s]"COL_GRAY": %s\n", /* CX_LOG_WARNING  */
        COL_WHITE"INF [%s]"COL_GRAY": %s\n",  /* CX_LOG_INFO     */
        COL_BLUE"DBG [%s]"COL_GRAY": %s\n",   /* CX_LOG_DEBUG    */
        "%s%s" /* CX_LOG_MARK: the tag placeholder must be present but tag
                               value will be ignored */
        "%s%s" /* CX_LOG_LAST: only for safety */
    };
    return CX_log_templates[level - CX_LOG_CRITICAL];
}

static int CX_log_trace_console(void *user_data, CX_LogLevel level,
                                const char *tag, const char *fmt, va_list ap)
{
    int ret = 0;
    bool is_dynbuf = false;
    /* flag: we must use a dynamic (larger than static) buffer? */
    char buf[CX_LOG_BUF_SIZE];
    char *msg = buf;
    size_t size = sizeof(buf);
    const char *template = NULL;

    tag = (tag != NULL) ?tag :"";
    /* CX_LOG_EXTRA special handling: force always empty tag */
    tag = (level != CX_LOG_MARK) ?tag :"";
    fmt = (fmt != NULL) ?fmt :"";
    template = log_template(level);
    
    size = CX_LOG_TEMPLATE_LEN + strlen(tag) + strlen(fmt) + 1;

    if (size > sizeof(buf)) {
        /* 
         * we use malloc/fprintf instead of CX_malloc because
         * we want custom error messages
         */
        msg = malloc(size);
        if (msg != NULL) {
            is_dynbuf = true;
        } else {
            fprintf(stderr, "(%s) CRITICAL: can't get memory in "
                    "CX_log_trace(), the output will be truncated.\n",
                    __FILE__);
            /* force reset to default values */
            msg = buf;
            size = sizeof(buf) - 1;
            ret = 1;
        }
    } else {
        size = sizeof(buf) - 1; // FIXME
    }

    /* construct real format string */
    snprintf(msg, size, template, tag, fmt);
    /* ant then finally deliver the message */
    vfprintf(user_data, msg, ap);

    if (is_dynbuf) {
        free(msg);
    }
    return ret;
}


static int CX_log_trace_file(void *user_data, CX_LogLevel level,
                             const char *tag, const char *fmt, va_list ap)
{
    /* FIXME: needs formatting */
    return CX_log_trace_console(user_data, CX_LOG_MARK, tag, fmt, ap);
}

static int CX_log_flush_file(CX_LogContext *ctx)
{
    int err = fflush(ctx->priv);
    if (err == EOF) {
        err = -1;
    }
    return err;
}

static int CX_log_close_file(CX_LogContext *ctx)
{
    /* the file ownership isn't yours, so we just
       want to make sure we delivered everything
    */
    return CX_log_flush_file(ctx);
}


static int CX_log_trace_null(void *user_data, CX_LogLevel level,
                             const char *tag, const char *fmt, va_list ap)
{
    return 0;
}

static int CX_log_close_null(CX_LogContext *ctx)
{
    return 0;
}

static int CX_log_flush_null(CX_LogContext *ctx)
{
    return 0;
}


CX_LogContext *CX_log_open_null()
{
    CX_LogContext *ctx = CX_zalloc(sizeof(CX_LogContext));
    if (ctx) {
        ctx->priv = NULL;
        ctx->max_level = -1;
        ctx->trace = CX_log_trace_null;
        ctx->close = CX_log_close_null;
        ctx->flush = CX_log_flush_null;
    }
    return ctx;
}

CX_LogContext *CX_log_open_file(CX_LogLevel max_level, FILE *sink)
{
    CX_LogLevel lev = CX_CLAMP(max_level, CX_LOG_ERROR, CX_LOG_MARK); /* TODO */
    CX_LogContext *ctx = NULL;

    if (sink) {
        ctx = CX_zalloc(sizeof(CX_LogContext));
    }
    if (ctx) {
        ctx->priv = sink;
        ctx->max_level = lev;
        ctx->trace = CX_log_trace_file;
        ctx->close = CX_log_close_file;
        ctx->close = CX_log_flush_file;
    }
    return ctx;
}


CX_LogContext *CX_log_open_console(CX_LogLevel max_level, FILE *sink)
{
    FILE *con = (sink) ?sink :stderr;
    CX_LogLevel lev = CX_CLAMP(max_level, CX_LOG_ERROR, CX_LOG_MARK); /* TODO */

    CX_LogContext *ctx = CX_zalloc(sizeof(CX_LogContext));
    if (ctx) {
        ctx->priv = con;
        ctx->max_level = lev;
        ctx->trace = CX_log_trace_console;
        ctx->close = CX_log_close_null;
        ctx->close = CX_log_flush_file;
    }
    return ctx;
}

CX_LogContext *CX_log_open_custom(CX_LogLevel max_level,
                                  CX_LogHandler log_handler,
                                  void *user_data)
{
    CX_LogLevel lev = CX_CLAMP(max_level, CX_LOG_ERROR, CX_LOG_MARK); /* TODO */

    CX_LogContext *ctx = CX_zalloc(sizeof(CX_LogContext));
    if (ctx) {
        ctx->priv = user_data;
        ctx->max_level = lev;
        ctx->trace = log_handler;
        ctx->close = CX_log_close_null;
        ctx->close = CX_log_flush_null;
    }
    return ctx;
}


int CX_log_trace(CX_LogContext *ctx,
                 CX_LogLevel level, const char *tag, const char *fmt, ...)
{
    int err = 0;
    va_list args;

    va_start(args, fmt);
    err = CX_log_trace_va(ctx, level, tag, fmt, args);
    va_end(args);

    return err;
}

int CX_log_trace_va(CX_LogContext *ctx,
                    CX_LogLevel level, const char *tag, const char *fmt,
                    va_list args)
{
    int err = 0;

    if (!ctx || !fmt || !tag) {
        return -1;
    }

    level = CX_CLAMP(level, CX_LOG_ERROR, CX_LOG_MARK); /* TODO */

    if (ctx->max_level >= level) {
        err = ctx->trace(ctx->priv, level, tag, fmt, args);
    }
    return err;
}

int CX_log_flush(CX_LogContext *ctx)
{
    if (!ctx) {
        return -1;
    }
    return ctx->flush(ctx);
}

int CX_log_close(CX_LogContext *ctx)
{
    int err = -1;
    if (ctx) {
        err = ctx->close(ctx);
        if (!err) {
            CX_free(ctx);
        }
    }
    return err;
}



/* vim: et sw=4 ts=4: */

