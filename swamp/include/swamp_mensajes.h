#ifndef SWAMP_MENSAJES_H
#define SWAMP_MENSAJES_H

#define PAQUETE 1

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include "sharedUtils.h"
#include "server.h"
#include "swap_memoria.h"
#include "archivos.h"
#include "swamp.h"
#include "codigos_operacion.h"


#define FALSE 0
#define TRUE 1

void atender_memoria(void);
void iniciar_carpincho(void);
void pedir_pagina(void);
void escribir_pagina(void);
void reservar_paginas(void);
void finalizar_carpincho(void);
void finalizar_conexion_con_memoria(void);

void handle_mensaje_simples(t_result*);
void handle_handshake_memoria(t_result*);
void handle_pedir_memoria(t_result*);
void handle_almacenar_memoria(t_result *);

void handle_mensajes(t_mensajes * mensajes);

void iterator( char* value);
t_list* recibir_paquete_utils(int socket_cliente);

void* recibir_buffer_utils(int* size, int socket_cliente);
void handle_finalizar_conexion_con_memoria(t_mensajes * mensajes);

void handle_asignacion(t_mensajes * mensajes );
void handle_pedir_pagina(t_mensajes * mensajes);
void handle_iniciar_proceso(t_mensajes * mensajes);
void handle_finalizar_proceso(t_mensajes * mensajes);
void handle_guardar_pagina(t_mensajes * mensajes);
void handle_escribir_pagina(t_mensajes * mensajes);

#endif