#include "archivos.h"
#include "swamp.h"
#include <unistd.h>

void inicializar_swamp_files( void){

    log_info(logger_swamp, "Cargando Archivos de las particiones(files)");

	  log_info(logger_swamp, "inicializando estructuras" );
	//printf("Cargando configuracion ...\n");

	t_list_iterator * iterador = list_iterator_create( (global_config_swamp->ARCHIVOS_SWAP) );


	listado_particiones = list_create();

	
	while(list_iterator_has_next(iterador) ){
		
		char * buffer;

		t_estructura_administrativa * particion = malloc(sizeof(t_estructura_administrativa));

		buffer =  list_iterator_next( iterador) ;

		cargar_file(buffer); 

		particion->cantidad_paginas_actuales =  global_total_paginas;

		particion->ruta_archivo = string_duplicate( buffer);	

		particion->puntero_mmap =  mapeo_archivo(buffer);   // RETURN EL PUNTERO AL BLOQUE MAPPER ---> USAR MSYNC 

		particion->listado_marcos = inicializar_listado_marcos_estructura_administrativa();

		particion->tamanio_bytes = 0;

		particion->tamanio_reservado = 0;
		//inicializo mapa de bits
		particion->mapa_de_bits=malloc(sizeof(particion->mapa_de_bits) * GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION);
		for (int i=0;i<GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION;i++){
			particion->mapa_de_bits[i]=LIBRE;
		}
		particion->marcos_libres=GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION;
		/////////

		
		rellenar_bloque_mapper_caracter( particion->puntero_mmap  );

		list_add( listado_particiones, particion );

	}

	list_iterator_destroy( iterador );
	
	log_info(logger_swamp, "Configuracion inicial de las particiones cargadas ");
}

t_list * inicializar_listado_marcos_estructura_administrativa( void ){

	int i;

	t_list * listado_marcos_nuevos = list_create();
	for(i=0; i<GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION; i++){
		t_marco * marco = malloc(sizeof(t_marco));

		marco->pid=0;
		marco->ocupado =0;
		marco->presencia=0;

		list_add(listado_marcos_nuevos, marco);
	}

	return listado_marcos_nuevos;
}

void cargar_file(char * nombre){   

    int fd;

   	if ((fd=open( nombre, O_RDWR | O_CREAT , S_IRUSR|S_IWUSR)) == -1)
				{
						log_info(logger_swamp, "No se pudo abrir el archivo: %s\n", nombre );
				}else{

        log_info(logger_swamp, "Se cargo/creo file:  %s    ", nombre);

                }

    truncar_archivo(nombre);

	rellenar_archivo_caracter(nombre, fd);

	close(fd);
}


char *  mapeo_archivo( char * ruta ){   //---HAY VARIOS ARCHIVOS TENER EN CUENTA! ----

		char * bloque_memoria;

		int fd_bloks = open(ruta, O_RDWR,  S_IRUSR | S_IWUSR );

			if(fd_bloks==-1){
				printf("error al abrir %s ", ruta);
				exit(-1);
			}

			bloque_memoria = (char *) mmap(0, global_config_swamp->TAMANIO_SWAP,  PROT_READ | PROT_WRITE, MAP_SHARED, fd_bloks ,0);

			if(bloque_memoria==MAP_FAILED){
				log_error(logger_swamp, "ERROR AL MAPEAR EL BINARIO BLOCKS"  );
				exit(-1);
			}

		//char * bloques = malloc(10);
		//memset( bloques, '8', 10 );		
		//---------------INICIALIZAR EL MAPPER SI ES NUEVO OR CARGADO -----------------
		//	memcpy(bloque_memoria, bloques,10 );
		//-------------------------------------------------------
		//	msync(bloque_memoria, 10, MS_SYNC);

		close( fd_bloks  );


	return  bloque_memoria; 
}


char * leer_bloque_memoria_marco(char * mmap, uint32_t numero_marco){    //LECTURA DIRECTA

	char * buffer = malloc( global_config_swamp->TAMANIO_PAGINA );

	//msync(mmap, global_config_swamp->TAMANIO_SWAP, MS_SYNC);
	
	memcpy(buffer, mmap + numero_marco * (global_config_swamp->TAMANIO_PAGINA), global_config_swamp->TAMANIO_PAGINA );
	
	return buffer;
} 


void persistir_bloque_memoria_marco(void * buffer ,uint32_t numero_marco, char * mmap){ //PERSISTE DIRECTO
	
	memcpy(mmap + numero_marco * (global_config_swamp->TAMANIO_PAGINA), buffer, global_config_swamp->TAMANIO_PAGINA );
	
	msync(mmap, global_config_swamp->TAMANIO_SWAP, MS_SYNC);

	//return 0;
}

void borrar_bloque_memoria_marco(uint32_t numero_marco, char * mmap){  //BORRA DIRECTO 

	memset(mmap + numero_marco * (global_config_swamp->TAMANIO_PAGINA), '\0', global_config_swamp->TAMANIO_PAGINA );
	
	//memcpy(mmap + numero_marco * (global_config_swamp->TAMANIO_PAGINA), buffer, global_config_swamp->TAMANIO_PAGINA );
	
	msync(mmap, global_config_swamp->TAMANIO_SWAP, MS_SYNC);

} 


int verificar_existe_archivo(char *path){

		struct stat estado;
		
		int resultado  = stat(path , &estado);  // devuelve 0 si existe el archivo/directorio, y -1 si no existe el archivo/directorio

		return  resultado;
}


void rellenar_bloque_mapper_caracter(char * bloque_memoria){    // ----- FUNCION DE TESTING ------

	char * buffer = malloc(global_config_swamp->TAMANIO_SWAP);


	memset( buffer, '\0', global_config_swamp->TAMANIO_SWAP );   //'\0'


	memcpy(bloque_memoria, buffer,  global_config_swamp->TAMANIO_SWAP  );


	msync(bloque_memoria, global_config_swamp->TAMANIO_SWAP, MS_SYNC);


	free(buffer);
}


void rellenar_archivo_caracter(char * ruta, int file){

	char * buffer = malloc(global_config_swamp->TAMANIO_SWAP);

	memset( buffer, '\0', global_config_swamp->TAMANIO_SWAP );

	write(file, buffer,  global_config_swamp->TAMANIO_SWAP );

	free(buffer);
}


void truncar_archivo(char * ruta){

    truncate( ruta, global_config_swamp->TAMANIO_SWAP);  
}


char *  extraer_nombre_file(char * ruta){   

	char**  ppLista = NULL;

	ppLista =	string_split(ruta, "/" );

 	int index=0;
	
	while(ppLista[index] != NULL){

		index++;
	}

	int contador=index-1;

	if(  contador < 0 ){  contador = 0; }

	char * name = string_new();

	name =  ppLista[contador];

	return  name;
}


char * extraer_file_sin_extension(char * ruta){

	char**  ppLista = NULL;

	ppLista =	string_split(ruta, "/" );

 	int index=0;

	while(ppLista[index] != NULL){

		index++;
	}

	int contador=index-1;

	//if(  contador < 0 ){  contador = 0; }

	char * name = string_new();

	name =  ppLista[contador];

	char**  ppLista2 = NULL;

	ppLista2 =	string_split(name, "." );

	char * name2 = string_new();

	name2 =  ppLista2[0];

	free(name);

	return  name2;
}


char * extraer_numero_file(char * ruta){

	char**  ppLista = NULL;

	ppLista =	string_split(ruta, "/" );

 	int index=0;	

	while(ppLista[index] != NULL){

		index++;
	}

	int contador=index-1;

	//if(  contador < 0 ){  contador = 0; }

	char * name = string_new();

	name =  ppLista[contador];

	char**  ppLista2 = NULL;

	ppLista2 =	string_split(name, "." );

	char * name2 = string_new();

	name2 =  ppLista2[0];	

	char * sub = string_new();

	if(  string_starts_with(name2, "swap") == true ){

		 sub = string_substring(name2, 4, string_length(name2) - 4 );
	}

	free(name);

	return  sub;
}


char *  obtener_ruta_proceso_numero( int num_proceso){

//	char * ruta = string_new();

	t_list_iterator * iterador = list_iterator_create( listado_particiones );


	while(list_iterator_has_next(iterador) ){

		t_ruta_proceso * proceso =  list_iterator_next( iterador) ;

		if( proceso->numero_proceso == num_proceso ){

			list_iterator_destroy(iterador);				

			return  proceso->ruta_de_archivo;

		}
	}

	list_iterator_destroy(iterador);

	return NULL;
}


