#ifndef SWAMP_CONFIG_H
#define SWAMP_CONFIG_H

#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <stdbool.h>
#include "sharedUtils.h"
#include <commons/string.h>
#include "stdint.h"
#include "archivos.h"

typedef struct{
    char * IP;
    uint32_t PUERTO;
    uint32_t TAMANIO_SWAP;
    uint32_t TAMANIO_PAGINA;
    t_list * ARCHIVOS_SWAP;
    uint32_t MARCOS_POR_PROCESO;  
    uint32_t RETARDO_SWAP;
 } t_files_config;

t_files_config * global_config_swamp;  //TODO: GLOBAL CONFIG SWAMP

t_config* global_config;

t_log* logger_swamp;


void cargar_config_file( char * path_config_file);
void iniciar_logger(void);
void files_config_parser(t_config* config, t_files_config* file_config );
void liberadorArrays(char **array);
void finally_config(void);
void lista_destructor(void * elemento);

void liberar_global_config(void);

#endif