#include "swamp_config.h"



void cargar_config_file( char * path_config_file){

    
     global_config = config_create(path_config_file);

     global_config_swamp = malloc(sizeof(t_files_config));

    files_config_parser( global_config, global_config_swamp    );

    config_destroy(global_config);
}


void iniciar_logger(){        

         logger_swamp = log_create("./cfg/swamp.log", "SWAMP", true, LOG_LEVEL_INFO);

        log_info(logger_swamp, "Soy el swamp ! %s", mi_funcion_compartida());

       // log_destroy(logger);   
}



void  files_config_parser(t_config* config, t_files_config* file_config ) {


  log_info(logger_swamp, "Cargando file de configuracion cfg del swamp" );
    
    file_config->PUERTO   = strdup(config_get_string_value(config, "PUERTO"));

    file_config->RETARDO_SWAP   = config_get_int_value(config, "RETARDO_SWAP");

    file_config->TAMANIO_PAGINA   = config_get_int_value(config, "TAMANIO_PAGINA");

    file_config->MARCOS_POR_PROCESO = config_get_int_value(config, "MARCOS_POR_PROCESO");

	file_config->TAMANIO_SWAP   = config_get_int_value(config, "TAMANIO_SWAP");

    file_config->IP = strdup(config_get_string_value(config, "IP"));

    char** ppLista= config_get_array_value(config, "ARCHIVOS_SWAP");   //REFACTORING DEBE CARGAR INT NO CHARS

    int index=0;
    
    file_config->ARCHIVOS_SWAP = list_create();

	while(ppLista[index] != NULL){
  
			list_add(file_config->ARCHIVOS_SWAP, ppLista[index] );

		index++;
	}


    GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION = file_config->TAMANIO_SWAP  /   file_config->TAMANIO_PAGINA;        

    free(ppLista);
}


void liberadorArrays(char **array){

    int count=0;

    while(array[count] != NULL){
            free(array[count]);
            count++;
    }
    free(array);
}


void finally_config(void){

       log_destroy(logger_swamp);
}



void liberar_global_config(void){

    free( global_config_swamp->IP );


    list_destroy_and_destroy_elements(global_config_swamp->ARCHIVOS_SWAP , lista_destructor );

}

void lista_destructor(void * elemento){

    free(elemento);
}
