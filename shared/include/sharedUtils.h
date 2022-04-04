#ifndef SHAREDUTILS_H
#define SHAREDUTILS_H

#include <stdio.h>
#include <commons/log.h>
#include "codigos_operacion.h"
#include <stdbool.h>

char* mi_funcion_compartida();

t_log* init_logger(char* path_logger, char* module_name, t_log_level log_level);
int max (int x, int y);

#endif