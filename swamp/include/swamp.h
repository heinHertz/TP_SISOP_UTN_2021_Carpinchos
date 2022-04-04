#ifndef SWAMP_H
#define SWAMP_H
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "sharedUtils.h"
#include "tests.h"
#include "swamp_config.h"
#include "archivos.h"
#include "swap_memoria.h"
#include "swamp_mensajes.h"
#include "particiones.h"
#include "pruebas_swamp.h"
#include "arpa/inet.h"
//para manjar mejor el pcb//

t_list *lista_pcb;
////


int cliente;
int socket_memoria;
void init_swamp(void);
void finally_system_swamp(void);
void handle_client(t_result* result);
void escuchar_mensajes_socket(t_parameter* parametro);
void escuchar_mensajes_socket_server(void);
char* obtener_String(void* buffer,int bytes);
void mandar_mensajes_socket_memoria(void);
void crear_hilo_recepcion_memoria(void);
void crear_conexion_inicial_memoria(void);
void iniciar_conexion_memoria(void);


#endif