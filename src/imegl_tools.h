#ifndef __IMEGL_TOOLS_H_INCLUDED__
#define __IMEGL_TOOLS_H_INCLUDED__

#include <stdio.h>

static FILE *msg_output_stream = stdout;
static FILE *msg_error_stream = stderr;

#define log_msg(format, ...)                                    \
  fprintf(msg_output_stream, format "\n", ##__VA_ARGS__)
#define log_err(format, ...)                            \
  fprintf(msg_error_stream, format "\n", ##__VA_ARGS__)

#endif  /* __IMEGL_TOOLS_H_INCLUDED__ */
