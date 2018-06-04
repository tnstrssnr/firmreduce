#ifndef LOGGING_H
#define LOGGING_H

#include <ir_stats_structs.h>

void log_text(char* text);

void log_stats(ir_stats_t* stats);

void log_result(int result);

void init_logging(char* out_path);

#endif