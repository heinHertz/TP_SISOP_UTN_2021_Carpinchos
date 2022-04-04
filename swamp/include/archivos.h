#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include "swamp_config.h"
#include "stdint.h"
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>


#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <unistd.h>
#include "swap_memoria.h"
#include <string.h>

typedef struct {
    int numero_proceso;
    char * ruta_de_archivo;
} t_ruta_proceso;


typedef struct {
    int ocupado;    // 0 vacio , 1 o  cualquiero otro ocupado    la combinacion ocupado = 1  presencia = 0 indica RESERVADO
    int pid;
    int presencia; 
} t_marco;

uint32_t GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION;

typedef struct{  
    uint32_t cantidad_paginas_actuales; 
    char * puntero_mmap;
    char * ruta_archivo;
    uint32_t tamanio_bytes;   //TAMANIO bytes ocupados, miestras mas grande mas lleno, 0 significa vacio
    uint32_t tamanio_reservado;
    uint32_t marcos_libres;   // necesario?
    t_list * listado_marcos;//esto no deberia usarse ya que es mas optimo acceder desde el pcb
    uint8_t *mapa_de_bits;//convendria usar un vector ya que es mucho mas eficiente que la lista, y es como lo dan en la teoria
} t_estructura_administrativa;

//char * bloques_mapper[10];

uint32_t TIPO_ASIGNACION_MEMORIA;

//t_list * listado_procesos;

t_list  * listado_particiones;   //listado de todas las estructuras administrativas


void inicializar_swamp_files( void);
void cargar_file(char * nombre);
void truncar_archivo(char * nombre);
int verificar_existe_archivo(char *path);
void archivos_finally(void);
void rellenar_archivo_caracter(char * ruta, int file);
char *  obtener_ruta_proceso_numero( int num_proceso);

char *  extraer_nombre_file(char * ruta);
char * extraer_numero_file(char * ruta);
char * extraer_file_sin_extension(char * ruta);

void liberar_lista(void* elemento);

char * mapeo_archivo( char * ruta );
void rellenar_bloque_mapper_caracter(char * bloque_memoria);


t_list * inicializar_listado_marcos_estructura_administrativa( void );

void persistir_bloque_memoria_marco(void * buffer ,uint32_t numero_marco, char * mmap);
void borrar_bloque_memoria_marco(uint32_t numero_marco, char * mmap);

char * leer_bloque_memoria_marco(char * mmap, uint32_t numero_marco);

#endif