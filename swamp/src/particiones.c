#include "particiones.h"

int inicializar_proceso_en_el_sistema_particiones(int pid){   
    //creo un pcb y lo agrego a la lista
    log_info(logger_swamp,"inicia un carpincho nuevo con id %i",pid);
    t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid=pid;
    pcb->archivo=NULL;
    pcb->tabla_de_paginas=NULL;
    pcb->framesUsados=0;
    list_add(lista_pcb,pcb);
    return 0;

    //no necesito reservar paginas, porque el carpincho no pidio nada aun
    /*
    int resultado =0;
    resultado = tiene_sistema_swamp_proceso_numero_pid(pid);
        if(resultado != 0){
            return 1;
        }

        t_estructura_administrativa * particion = (t_estructura_administrativa *)buscar_particion_con_menor_espacio_disponible();
        reservar_proceso_nuevo_a_particiones(particion, pid);

    return resultado;
    */
}


int reservo_paginas(uint32_t pid,uint32_t cant){
    t_pcb *pcb=buscar_pcb(pid);
    if (pcb->archivo==NULL){//es el primer alloc
        pcb->archivo=buscar_particion_con_menor_espacio_disponible();
        log_info(logger_swamp,"al carpincho %i le asigno el archivo %s",pcb->pid,pcb->archivo->ruta_archivo);
    }
    log_info(logger_swamp,"voy a reservar %i paginas para el carpincho %i",cant,pid);
    if (pcb->archivo->marcos_libres < cant){
        log_info(logger_swamp,"el archivo no tiene lugar suficiente",cant,pid);
        return -1;//el archivo no tiene marcos suficientes
     }
    
    pcb->archivo->marcos_libres = pcb->archivo->marcos_libres - cant;
    int *temp=malloc(sizeof(int)*cant);
    int j=0; 
    for(int i=0;i < GLOBAL_CANTIDAD_DE_MARCOS_POR_PARTICION && j< cant;i++){
        if (pcb->archivo->mapa_de_bits[i]==LIBRE){
            pcb->archivo->mapa_de_bits[i]=OCUPADO;
            temp[j]=i;
            j++;
        }
    }
    int *new = malloc(sizeof(int)* (pcb->framesUsados + cant));
    for (int i=0;i<pcb->framesUsados;i++){
        new[i]=pcb->tabla_de_paginas[i];
    }
    
    for (int i=pcb->framesUsados;i<pcb->framesUsados + cant;i++){
        new[i]=temp[i-pcb->framesUsados];
    }
    pcb->framesUsados=pcb->framesUsados + cant;
    free(pcb->tabla_de_paginas);
    pcb->tabla_de_paginas=new;
    free(temp);
    log_info(logger_swamp,"paginas reservadas exitosamente");
    return 0;
}

t_pcb *buscar_pcb(uint32_t pid){
    if(list_size(lista_pcb)==0){
        return 0;
    }
    t_pcb *pcb;
    for(int i=0;i < list_size(lista_pcb);i++){
        pcb=list_get(lista_pcb,i);
        if (pcb->pid==pid){return pcb;}
    }
}


int guardar_marco_en_swamp( int pid, char * buffer){

   return agregar_proceso_a_particion(pid, buffer); //hex ok

}

// busca una particion y agrega el proceso pid  y u n buffer
int agregar_proceso_a_particion(uint32_t pid, char * buffer){

	t_list_iterator * iterador_particion = list_iterator_create(  listado_particiones );

	int indice=0; // es un BOOL
	while(list_iterator_has_next(iterador_particion) ){		//comprueba que existe el proceso
		
		t_estructura_administrativa * particion =  list_iterator_next( iterador_particion) ;

        t_list_iterator * iterador_marcos = list_iterator_create(  particion->listado_marcos );

            while(list_iterator_has_next(iterador_marcos) ){

                t_marco * marco =  list_iterator_next( iterador_marcos) ;

                    if( marco->pid == pid ){  //significa que existe el proceso dentro de las particiones
                        indice = 1;
                    // asignar_memoria(particion, pid);  /// GUARDAR DIRECTAMENTE
                        list_iterator_destroy( iterador_marcos );
                        list_iterator_destroy( iterador_particion );
                       return agregar_proceso_viejo_a_particiones(particion, pid, buffer);
                       // break;
                }
            }       

	    list_iterator_destroy( iterador_marcos );
	}

    list_iterator_destroy( iterador_particion );

    if( indice == 0 ){  // significa que no existe el proceso, es nuevo , se agrega a la particion con mas lugar disponible
     
        t_estructura_administrativa * particion = (t_estructura_administrativa *)buscar_particion_con_menor_espacio_disponible();
        return agregar_proceso_nuevo_a_particiones(particion, pid, buffer);
    }

    return -1;
}


int agregar_proceso_nuevo_a_particiones(t_estructura_administrativa * particion ,uint32_t pid, char * buffer ){

    if(TIPO_ASIGNACION_MEMORIA == TIPO_ASIGNACION_FIJA ){
            
            t_list_iterator * iterador_marco = list_iterator_create(  particion->listado_marcos );

            int index = 0;
            int indice=0;
            while(list_iterator_has_next(iterador_marco) ){

                t_marco * marco =  list_iterator_next( iterador_marco);

             /*    if( indice != 0 && indice<global_config_swamp->MARCOS_POR_PROCESO){
                    marco->ocupado = 1;                                    
                    indice++;               
                 }*/
                 //if(indice<global_config_swamp->MARCOS_POR_PROCESO){  break; return; }

                if( marco->pid == pid && marco->presencia == 0){ // if( marco->pid == pid  && indice == 0 && marco->ocupado == 0 ){
                    indice++;

                    marco->pid = pid;
                    marco->ocupado = 1;
                    marco->presencia=1;
                    particion->tamanio_bytes = particion->tamanio_bytes + global_config_swamp->TAMANIO_PAGINA; 
                    
                    persistir_bloque_memoria_marco( buffer , index,    particion->puntero_mmap);
                  
                    list_iterator_destroy( iterador_marco ); 
                    return index;
                }       
                index++;
            }
             list_iterator_destroy( iterador_marco ); 
    }  
    

    if( TIPO_ASIGNACION_MEMORIA == TIPO_ASIGNACION_DINAMICA){

        t_list_iterator * iterador_marco = list_iterator_create(  particion->listado_marcos );

            int index = 0;
            while(list_iterator_has_next(iterador_marco) ){

                t_marco * marco =  list_iterator_next( iterador_marco);

                if( marco->pid == 0  ){

                    marco->pid = pid;
                    marco->ocupado = 1;
                    marco->presencia=1;
                    particion->tamanio_bytes = particion->tamanio_bytes + global_config_swamp->TAMANIO_PAGINA; 
                    
                     //particion->listado_marcos
                    persistir_bloque_memoria_marco( buffer , index,    particion->puntero_mmap);
                   
                    list_iterator_destroy( iterador_marco ); 
                    return index;
                } 
                index++;
            }
             list_iterator_destroy( iterador_marco ); 
    }

    return -1;
}


t_estructura_administrativa * buscar_particion_con_menor_espacio_disponible(void){

         t_list_iterator * iterador_particion = list_iterator_create(  listado_particiones );
            t_estructura_administrativa *part;
            int min_marcos_libres=0;
           // int indice_de_la_particion_menor_tamanio_disponible;
            while(list_iterator_has_next(iterador_particion) ){
            
                t_estructura_administrativa * particion =  list_iterator_next( iterador_particion);
                if( min_marcos_libres < particion->marcos_libres ){
                    part=particion;
                    min_marcos_libres= particion->marcos_libres;
                }
            }

            list_iterator_destroy( iterador_particion );
            
            return part;
}


int agregar_proceso_viejo_a_particiones(t_estructura_administrativa * particion ,uint32_t pid, char * buffer ){

      if(TIPO_ASIGNACION_MEMORIA == TIPO_ASIGNACION_FIJA ){

            t_list_iterator * iterador_marco = list_iterator_create(   particion->listado_marcos );

            int index = 0;
           
            while(list_iterator_has_next(iterador_marco) ){

                t_marco * marco =  list_iterator_next( iterador_marco);
                
                if(marco->pid == pid && marco->presencia == 0) { // if( marco->pid == 0 && indice !=0 ){

                    marco->pid = pid;
                    marco->ocupado = 1;
                    marco->presencia=1;
                    particion->tamanio_bytes = particion->tamanio_bytes + global_config_swamp->TAMANIO_PAGINA;   

                    persistir_bloque_memoria_marco( buffer , index,    particion->puntero_mmap);
                
                    list_iterator_destroy( iterador_marco );
                    return index;
                } 

              //  if(marco->pid == pid && presencia == 0) {
               //     indice=1;
              //  }
                index++;
            }
             list_iterator_destroy( iterador_marco ); 
        }  


         
    if( TIPO_ASIGNACION_MEMORIA == TIPO_ASIGNACION_DINAMICA){    

         // t_list * lista_marco = list_create( particion->listado_marcos );

         t_list_iterator * iterador_marco = list_iterator_create(   particion->listado_marcos );

            int index = 0;
            while(list_iterator_has_next(iterador_marco) ){

                t_marco * marco =  list_iterator_next( iterador_marco);                
                            

                if( marco->pid == 0   ||  ( marco->pid == pid  && marco->presencia == 0 ) ){

                    marco->pid = pid;
                    marco->ocupado = 1;
                    marco->presencia=1;
                    particion->tamanio_bytes = particion->tamanio_bytes + global_config_swamp->TAMANIO_PAGINA;   

                    persistir_bloque_memoria_marco( buffer , index,    particion->puntero_mmap);
                   
                     list_iterator_destroy( iterador_marco ); 
                    return index;
                } 
                index++;
            }
             list_iterator_destroy( iterador_marco ); 
        }  

    return -1;  //si no retorna antes es porque no hay lugar y sale NULL
}


/// SE ASUME QUE EL PROCESO NO ESTA EN EL SISTEMA DE PARTICIONES,  RETORNA 0 SI TODO OK  ,  RETORNA 1 SI EL PROCESO YA ESTABA EN SWAMP
//esta funcion es equivalente a reservar pagina en ASIGNACION DINAMICA



void reservar_proceso_nuevo_a_particiones(t_estructura_administrativa * particion ,uint32_t pid ){

   if(TIPO_ASIGNACION_MEMORIA == TIPO_ASIGNACION_FIJA ){
             
           t_list_iterator * iterador_marco = list_iterator_create(  particion->listado_marcos );

            int index = 0;
            int indice=0;
            
            while(list_iterator_has_next(iterador_marco) ){

                t_marco * marco =  list_iterator_next( iterador_marco);

                 if( indice != 0 && indice<global_config_swamp->MARCOS_POR_PROCESO){
                    marco->ocupado = 1; 
                    marco->presencia =0; 
                    marco->pid=pid;                                
                    indice++;               
             }
                 //if(indice<global_config_swamp->MARCOS_POR_PROCESO){  break; return; }

                if( marco->pid == 0 && indice==0 &&  marco->presencia == 0 ){
                    indice++;

                    marco->pid = pid;
                    marco->ocupado = 1;
                    marco->presencia=0;
                    particion->tamanio_bytes = particion->tamanio_bytes + global_config_swamp->TAMANIO_PAGINA * global_config_swamp->MARCOS_POR_PROCESO; 
                    
                  // persistir_bloque_memoria_marco( buffer , index,  pid,  particion->puntero_mmap);
                  
                  }       
                index++;
            }
             list_iterator_destroy( iterador_marco ); 
    }  
    

    if( TIPO_ASIGNACION_MEMORIA == TIPO_ASIGNACION_DINAMICA){

        t_list_iterator * iterador_marco = list_iterator_create(  particion->listado_marcos );

            int index = 0;
            while(list_iterator_has_next(iterador_marco) ){

                t_marco * marco =  list_iterator_next( iterador_marco);

                if( marco->pid == 0 ||  ( marco->pid == pid  && marco->presencia == 0 )  ){

                    marco->pid = pid;
                    marco->ocupado = 1;
                    marco->presencia=0;
                    particion->tamanio_bytes = particion->tamanio_bytes + global_config_swamp->TAMANIO_PAGINA; 
                    
                    
                    //persistir_bloque_memoria_marco( buffer , index,  pid,  particion->puntero_mmap);
                   
                    list_iterator_destroy( iterador_marco ); 
                    return;
                } 
                index++;
            }
             list_iterator_destroy( iterador_marco ); 
    }
}

void finalizar_proceso(uint32_t pid){    
    t_pcb *pcb=buscar_pcb(pid);
    quitar_de_lista(lista_pcb,pcb);
    for(int i=1;i<pcb->framesUsados;i++){//marco los frames que tenia el proceso como libres
        pcb->archivo->mapa_de_bits[pcb->tabla_de_paginas[i]]=LIBRE;
    }
    free(pcb->tabla_de_paginas);
    free(pcb);

}

void quitar_de_lista(t_list *lista, void *ptr){
    t_list *temp;
    for (int i=0;i<list_size(lista);i++){
        temp=list_get(lista,i);
        if (ptr==temp){
            list_remove(lista,i);
            return;
        }
    }
}

void escribir_pagina_proceso(int pid, int numero_pagina, char * buffer){
    t_pcb *pcb=buscar_pcb(pid);

   // t_estructura_administrativa * particion = buscar_particion_por_pid(pid);

    persistir_bloque_memoria_marco( buffer , pcb->tabla_de_paginas[numero_pagina],  pcb->archivo->puntero_mmap);

}

char * leer_marco_proceso(uint32_t pid, uint32_t numero_pagina){
        t_pcb *pcb=buscar_pcb(pid);

       //t_estructura_administrativa * particion = buscar_particion_por_pid(pid);

        return leer_marco(pcb->archivo, pcb->tabla_de_paginas[numero_pagina]);
}

char * leer_marco(t_estructura_administrativa * particion, uint32_t numero_marco){

    return leer_bloque_memoria_marco(particion->puntero_mmap, numero_marco);

}

//la diferencia entre borrar marco y vaciar, es que vaciar solamente pone en 0 los dato del marco
void vaciar_marco(t_estructura_administrativa * particion, uint32_t numero_marco){

       t_marco * marco =  list_get(  particion->listado_marcos , numero_marco ); 
       marco->presencia=0;

        borrar_bloque_memoria_marco( numero_marco, particion->puntero_mmap);  //BORRA DIRECTO
}


//BORRAR TODO EL PID , ELIMINAR EL MARCO Y LA CONESION CON UN PROCESO DEL SISTEMA
void borrar_marco(t_estructura_administrativa * particion, uint32_t numero_marco){

       t_marco * marco =  list_get(  particion->listado_marcos , numero_marco );
       marco->ocupado = 0;
       marco->pid = 0;
       marco->presencia=0;

        borrar_bloque_memoria_marco( numero_marco, particion->puntero_mmap);  //BORRA DIRECTO
}


void borrar_todos_los_marcos_de_un_proceso(uint32_t pid){

    t_list_iterator * iterador_particion = list_iterator_create(  listado_particiones );

	int index;
	while(list_iterator_has_next(iterador_particion) ){		//comprueba que existe el proceso
		
		t_estructura_administrativa * particion =  list_iterator_next( iterador_particion) ;

        t_list_iterator * iterador_marcos = list_iterator_create(  particion->listado_marcos );
            index =0;
            while(list_iterator_has_next(iterador_marcos) ){

                t_marco * marco =  list_iterator_next( iterador_marcos) ;

                    if( marco->pid == pid ){  //significa que existe el proceso dentro de las particiones
                       marco->pid = 0;
                       marco->ocupado = 0;
                       marco->presencia=0;
                       borrar_marco(particion, index);
                   // break;
                }
                index++;
            }       

	    list_iterator_destroy( iterador_marcos );

	}

    list_iterator_destroy( iterador_particion );
}


char * pedir_marco_particion(t_estructura_administrativa * particion, uint32_t numero_marco){

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);

	//msync(mmap, global_config_swamp->TAMANIO_SWAP, MS_SYNC);

	memcpy(buffer, (particion->puntero_mmap) + (global_config_swamp->TAMANIO_PAGINA) * numero_marco  , global_config_swamp->TAMANIO_PAGINA );
	
    return buffer;
}


t_estructura_administrativa * buscar_particion_por_pid(uint32_t pid ){

	t_list_iterator * iterador_particion = list_iterator_create(  listado_particiones );

	int indice=0; 
	while(list_iterator_has_next(iterador_particion) ){		//comprueba que existe el proceso
		
		t_estructura_administrativa * particion =  list_iterator_next( iterador_particion) ;
        
       indice = particion_tiene_proceso_pid(particion, pid);

        if(indice != 0){
            list_iterator_destroy( iterador_particion );
                return particion;
        }
    }

    list_iterator_destroy(iterador_particion);
    return NULL;
}


// RETURN 0 SI NO, RETURN 1 SI EXISTE
int  particion_tiene_proceso_pid(  t_estructura_administrativa * particion, uint32_t pid){
      
	t_list_iterator * iterador_marcos = list_iterator_create(   particion->listado_marcos  );

	int indice=0;
	while(list_iterator_has_next(iterador_marcos) ){	
             t_marco * marco =  list_iterator_next( iterador_marcos);

             if( marco->pid == pid){   //no es la primera vez
                 indice = 1;
               
             }
             if(indice == 1 ){
                 list_iterator_destroy(iterador_marcos);
                 return 1;
             }
        }
        list_iterator_destroy(iterador_marcos);
    return indice;
}


///RETURN 0 SI NO EXISTE EN EL SISTEMA,  RETORNA 1 (TRUE)  SI YA EXISTE EN EL SISTEMA EL PROCESO DE PID
int tiene_sistema_swamp_proceso_numero_pid(int pid){

	t_list_iterator * iterador_particion = list_iterator_create(  listado_particiones );

	int indice=0; // es un BOOL
	while(list_iterator_has_next(iterador_particion) ){		//comprueba que existe el proceso
		
		t_estructura_administrativa * particion =  list_iterator_next( iterador_particion) ;
        
       indice = particion_tiene_proceso_pid(particion, pid);

        if(indice != 0){
            list_iterator_destroy(iterador_particion);
                return 1;
        }
    }
    list_iterator_destroy(iterador_particion);
    return indice;
}


void test_marcos(t_estructura_administrativa * particion){

    int index =0;
    t_list_iterator * iterador_proceso1 = list_iterator_create( particion->listado_marcos);

        while(list_iterator_has_next(iterador_proceso1) ){

            t_marco * marco =  list_iterator_next( iterador_proceso1) ;
            printf("%d \t marco: %d,  \t ocupado : \t %d \t presencia: %d \n", index, marco->pid, marco->ocupado, marco->presencia );

        index++;
        }
}


void test_marcos_del_sistema(void){

    int partic =0;
    t_list_iterator * iterador_particion = list_iterator_create( listado_particiones);

    int index =0;
    while(list_iterator_has_next(iterador_particion) ){

            t_estructura_administrativa * particion =  list_iterator_next( iterador_particion);

            t_list_iterator * iterador_marcos = list_iterator_create( particion->listado_marcos);
            printf("particion: %d\n", partic);

            while(list_iterator_has_next(iterador_marcos) ){

                t_marco * marco =  list_iterator_next( iterador_marcos) ;
                printf("marco: %d \t pid: %d,  \t ocupado : \t %d \t presencia: %d \n", index, marco->pid, marco->ocupado, marco->presencia );

            index++;
            }
            list_iterator_destroy(iterador_marcos);
        index=0;
        partic++;
    }

    list_iterator_destroy(iterador_particion);

}

void particiones_finally(void){


	list_destroy_and_destroy_elements(listado_particiones, liberar_lista);
	 
}

void liberar_lista(void* elemento)
{

	t_estructura_administrativa * particion = (t_estructura_administrativa *) elemento;

	munmap( particion->puntero_mmap , global_config_swamp->TAMANIO_PAGINA );  //DESMAPEAR

  //  free( particion->puntero_mmap );

    free( particion->mapa_de_bits);

    free(particion->ruta_archivo);
    
    list_destroy_and_destroy_elements(particion->listado_marcos, liberar_marcos_particion);

    free(elemento);
}

void liberar_marcos_particion(void * elemento){

    free(elemento);
}