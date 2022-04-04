#include "swap_memoria.h"

void  inicializar_swap(void){
    
    int i;

    global_total_paginas = global_config_swamp->TAMANIO_SWAP  / global_config_swamp->TAMANIO_PAGINA;

   

	 paginas_swap  = list_create();


    for(i=0; i<global_total_paginas;i++){

        t_pagina * pagina = malloc(sizeof(t_pagina));

        pagina->free = 0; // 0 LIBRE   1 OCUPADO

        pagina->posicion = i;

        pagina->pagina = NULL;

        list_add(paginas_swap, pagina );
    }

}

/*
t_pagina * pedir_pagina_swap(){    //PIDE PRIMER PAGINA LIBRE EN LA LISTA ENLAZADA

	t_list_iterator * iterador = list_iterator_create( global_total_paginas );

    t_pagina * pagina_resultado = NULL;


	while(list_iterator_has_next(iterador) ){
		
		t_pagina  * pagina =  list_iterator_next( iterador) ;

        if(pagina->free == 0 ){

            pagina_resultado = pagina;
        }
	}

 list_iterator_destroy(iterador);
return pagina_resultado;
}
*/


void guardar_pagina( int proceso ){  // int numero_pagina, void * buffer , int tamanio_buffer){

    char * ruta;

    ruta = obtener_ruta_proceso_numero(proceso);

    int fd;

    printf(" ---------  %s  ------------   ",  ruta);

    if ((fd=open( ruta, O_RDWR  , S_IRUSR|S_IWUSR)) == -1)
            {
                    log_info(logger_swamp, "No se pudo abrir el archivo: %s ", ruta );
            }

	close(fd);

    free(ruta);

}




void borrar_pagina(){

}