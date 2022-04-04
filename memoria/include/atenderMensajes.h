#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include "sharedUtils.h"
#include "server.h"
#include "codigos_operacion.h"
#define FALSE 0
#define TRUE 1

int cliente_swamp;

void handle_client(t_result* result);
void handle_memalloc(t_result* result);
void handle_memfree(t_result* result);
void handle_memread(t_result* result);
void handle_memwrite(t_result* result);
char* notificar_tipo_asigancion();
void conectar_swap(ops_code tipoDeMensaje,uint32_t pid);
