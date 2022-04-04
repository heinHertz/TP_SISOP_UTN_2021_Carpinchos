#ifndef PARTICIONES_H
#define PARTICIONES_H

#include <stdio.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include "codigos_operacion.h"
#include "archivos.h"
#include "swamp.h"

typedef struct{
    int pid;
    int framesUsados;//la cantidad de frames que tiene asignados
    int *tabla_de_paginas;//aca guardo el frame al que apuntan las paginas del proceso
    t_estructura_administrativa *archivo;//el archivo donde esta guardado
}t_pcb;

typedef struct
{
	uint32_t pid;
    uint32_t cantidad_marcos;
    t_list * marcos;
} t_proceso;


t_list * listado_procesos;


int reservo_paginas(uint32_t pid,uint32_t cant);
t_pcb *buscar_pcb(uint32_t pid);

t_estructura_administrativa * buscar_particion_con_menor_espacio_disponible(void);

int agregar_proceso_viejo_a_particiones(t_estructura_administrativa * particion ,uint32_t pid, char * buffer );
int guardar_marco_en_swamp(int pid, char * buffer);
int agregar_proceso_nuevo_a_particiones(t_estructura_administrativa * particion ,uint32_t pid, char * buffer );

void borrar_marco(t_estructura_administrativa * particion, uint32_t numero_marco);
void borrar_todos_los_marcos_de_un_proceso(uint32_t pid);
void test_marcos_del_sistema(void);

int tiene_sistema_swamp_proceso_numero_pid(int pid);
int  particion_tiene_proceso_pid(  t_estructura_administrativa * particion, uint32_t pid);
int inicializar_proceso_en_el_sistema_particiones(int pid);
void reservar_proceso_nuevo_a_particiones(t_estructura_administrativa * particion ,uint32_t pid );
char * pedir_marco_particion(t_estructura_administrativa * particion, uint32_t numero_marco);

char * leer_marco_proceso(uint32_t pid, uint32_t numero_pagina);
t_estructura_administrativa * buscar_particion_por_pid(uint32_t pid );
char * leer_marco(t_estructura_administrativa * particion, uint32_t numero_marco);
void escribir_pagina_proceso(int pid, int numero_pagina, char * buffer);

void finalizar_proceso(uint32_t pid);
void liberar_lista(void* elemento);
void particiones_finally(void);
void liberar_marcos_particion(void * elemento);

int agregar_proceso_a_particion(uint32_t pid, char * buffer);

void quitar_de_lista(t_list *lista, void *ptr);
#endif