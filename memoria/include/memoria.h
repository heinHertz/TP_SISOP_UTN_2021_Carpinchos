#ifndef MEMORIA_H
#define MEMORIA_H
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include "sharedUtils.h"
#include <signal.h>
#include <string.h>
#include <semaphore.h>
#include <assert.h>
#include "tipos.h"
#include "tests.h"
#include "atenderMensajes.h"  //Por handle de memoria
#include "memoriaPrincipal.h"
#include "manejoMemoria.h"
#include "funcionesDeLlamado.h"
#include "codigos_operacion.h"


t_memoria_config* memoria_config;
t_log * logger;
t_log *logram;


char string_log[1024*10];

//SEMAFOROS
sem_t sem_inst;
sem_t sem_list;
sem_t sem_tlb;
sem_t sem_bitmap;
sem_t sem_ram;
sem_t sem_tablas;
sem_t sem_swamp;
sem_t sem_fallo;
pthread_mutex_t MX_SEND;
pthread_mutex_t MX_RECV;

//Variables Globales
 
 int socketSwamp;
 int paginasMaximas;
 u_long instante;
 t_list *listaContadores;
 t_tabla_paginas **frames;

 char* puntero_memoria_principal;
 int tamanioBitMapPrincipal;
 uint32_t cantFrames;
 char* punteroBitMap;
 t_bitarray* bitMap;
 int *mapaDeBits;
 t_list* tablaDeProcesos;
 t_tlb *TLB;
 t_list *listaPCB;
 
uint32_t tothit;
uint32_t totmiss;
// FUNCIONES
int socket_server;
void sig_handler(int signum);
void memoria_init(t_memoria_config** memoria_config);
void memoria_finally(t_memoria_config* memoria_config, t_log* logger);
t_memoria_config* memoria_config_loader(char* path_config_file);
void memoria_config_parser(t_config* config, t_memoria_config* memoria_config);
void memoria_destroy(t_memoria_config* memoria_config);


void iniciarMemoriaPrincipal(t_memoria_config* memoria_config);


void  enviar_swamp_asignacion_memoria(void);
void handle_conexion_inicial_swamp(int socket);
void escuchar_mensajes_socket_swamp(void );
void * handler_respuestas_swamp(t_mensajes * mensajes );
void finalizar_conexion_swamp();
void * pedir_pagina_swamp(int numero, int pid);
int almacenar_pagina_swamp(char * buffer, int tamanio, int pid);
void * finalizar_proceso_swamp(int pid);
void * escribir_pagina_swamp(int pid, int numero_pagina, void * buffer);
void * procesar_respuesta_pedir_pagina(t_mensajes * mensajes);
int  guardar_pagina_swamp(int pid, void * buffer);
int  iniciar_proceso_swamp(int pid);

void hilo_mem_test1(void);
void hilo_mem_test2(int hilo);
void hilo_mem_test3(void);
void hilo_mem_test4(void);
void hilo_mem_test5(void);
void hilo_mem_test6(void);
void hilo_mem_test7(void);
void hilo_mem_test8(void);
void hilo_mem_test9(void);


void mem_test_1(void);
void mem_test_2(void);
void mem_test_3(void);
void mem_test_4(void);
void mem_test_5(void);
void mem_test_6(void);
void mem_test_7(void);
void mem_test_8(void);
void mem_test_9(void);


#endif