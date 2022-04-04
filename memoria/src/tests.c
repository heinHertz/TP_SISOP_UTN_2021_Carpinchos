#include "memoria.h"



  
    void prueba(){
 /*   tamswamp =128;
    swamp = malloc(tamswamp);
    bitMapSwamp;
   
    punteroBitMapS = malloc(max(1, tamswamp/memoria_config->TAMANIO_PAGINA));
    bitMapSwamp = bitarray_create_with_mode(punteroBitMapS, max(1,tamswamp/memoria_config->TAMANIO_PAGINA/8), MSB_FIRST);

    for (int i=0;i<tamswamp;i++){
        swamp[i]='s';
    }
	for(double i=0;  tamanioBitMapPrincipal> i; i++){

		bitarray_clean_bit(bitMap, i);		
	}
    */
    log_info(logger,"%i",sizeof(t_alloc));
    t_pcb *PCB = malloc(sizeof(t_pcb));
    char *leido = malloc (100);
    PCB->pid = 1;
    PCB->tabla_paginas = NULL;  
    nuevo_proceso_ram(PCB);
    t_pcb *PCB2 = malloc(sizeof(t_pcb));
    PCB2->pid = 2;
    PCB2->tabla_paginas = NULL; 
    nuevo_proceso_ram(PCB2);

    //log_info(logger,"%s",puntero_memoria_principal);
    int ptr1=alocar_memoria(1,PCB);
    int ptr2=alocar_memoria(1,PCB2);
    escribir_memoria("z",ptr1,1,PCB);
    escribir_memoria("W",ptr2,2,PCB2);
    leer_memoria(leido,ptr1,1,PCB);
    log_info(logger,"leo %c",leido[0]);

    leer_memoria(leido,ptr2,1,PCB2);
    log_info(logger,"leo %c",leido[0]);

    int ptr3=alocar_memoria(100,PCB2);

    escribir_memoria("Para facilitar el desarrollo del trabajo práctico vamos a detallar el set de tareas\0",17,1,PCB2);
    leer_memoria(leido,ptr3,85,PCB2);
    log_info(logger,"leo %s",leido);

/*    int ptr2=alocar_memoria(10,PCB);
    int ptr3=alocar_memoria(10,PCB);
    int ptr4=alocar_memoria(10,PCB);
    int ptr5=alocar_memoria(10,PCB);
    allocs(PCB);

    liberar_memoria(ptr1,PCB);
    allocs(PCB);
    liberar_memoria(ptr2,PCB);
    allocs(PCB);
    int ptr6=alocar_memoria(10,PCB);
    allocs(PCB);
    int ptr7=alocar_memoria(8,PCB);
    allocs(PCB);
    int ptr8=alocar_memoria(1,PCB);
    allocs(PCB);

    liberar_memoria(ptr3,PCB);
    allocs(PCB);

    liberar_memoria(ptr5,PCB);
    allocs(PCB);
    int ptr9=alocar_memoria(10,PCB);
    allocs(PCB);
    liberar_memoria(ptr4,PCB);
    allocs(PCB);
    

    escribir_memoria("ptr1\0",ptr1,5,PCB);
    escribir_memoria("ptr2\0",ptr2,5,PCB);
    escribir_memoria("ptr3\0",ptr3,5,PCB);
    escribir_memoria("ptr4\0",ptr4,5,PCB);
    escribir_memoria("ptr5\0",ptr5,5,PCB);
    allocs(PCB);
    for (int i=0;i<memoria_config->TAMANIO;i++){
        log_info(logger,"%c",puntero_memoria_principal[i]);
    }

    leer_memoria(leido,ptr1,5,PCB);
    log_info(logger,"leo %s",leido);
    leer_memoria(leido,ptr2,5,PCB);
    log_info(logger,"leo %s",leido);
    leer_memoria(leido,ptr3,5,PCB);
    log_info(logger,"leo %s",leido);
    leer_memoria(leido,ptr4,5,PCB);
    log_info(logger,"leo %s",leido);
    leer_memoria(leido,ptr5,5,PCB);
    log_info(logger,"leo %s",leido);




    liberar_memoria(ptr1,PCB);
    allocs(PCB);
    liberar_memoria(ptr5,PCB);
    allocs(PCB);
    liberar_memoria(ptr2,PCB);
    allocs(PCB);
    liberar_memoria(ptr4,PCB);
    allocs(PCB);
    liberar_memoria(ptr3,PCB);
    allocs(PCB);
  */  
}

void allocs(t_pcb *PCB){
    t_alloc heap;
    int pos=0;
    if (PCB->alocado ==0) {
        log_info(logger,"el pcb esta vacio");
        return;
    }
    int cont=1;
    leer_memoria(&heap,pos,tamHeap,PCB);
    log_info(logger,"HEAP:%i,pos %i, prev:%i, next:%i, free:%i",cont,pos,heap.prevAlloc,heap.nextAlloc,heap.isFree);
    
    while (heap.nextAlloc !=NULL){
        cont++;
        pos=heap.nextAlloc;
        leer_memoria(&heap,pos,tamHeap,PCB);
        log_info(logger,"HEAP:%i,pos %i, prev:%i, next:%i, free:%i",cont,pos,heap.prevAlloc,heap.nextAlloc,heap.isFree);
    }

    return;


}

int *reservar_frames_test(uint32_t cant,t_pcb *PCB){
    int *listaFrames = malloc(sizeof(int)*cant);
    for (int i=0;i<cant;i++){
        listaFrames[i]=i;
        bitarray_clean_bit(bitMapSwamp,i);
    }
    return listaFrames;
}

void *traer_pagina_test(u_int32_t nroPagina,t_pcb *PCB){
    char *contenidoSwap=malloc(memoria_config->TAMANIO_PAGINA);
    bitarray_set_bit(bitMapSwamp,nroPagina);
    memcpy(contenidoSwap,&swamp[nroPagina*memoria_config->TAMANIO_PAGINA],memoria_config->TAMANIO_PAGINA);
    return contenidoSwap;
}

uint32_t enviar_pagina_test(void *contenidoSwap,t_pcb *PCB){
    for (int i=0;i<tamswamp/memoria_config->TAMANIO_PAGINA;i++){
        if(bitarray_test_bit(bitMapSwamp,i)==1){
            bitarray_clean_bit(bitMapSwamp,i);
             memcpy(&swamp[i*memoria_config->TAMANIO_PAGINA],contenidoSwap, memoria_config->TAMANIO_PAGINA);
            return i;
        }
    }
}
void print_ram(){
    for (int i=0;i<memoria_config->TAMANIO;i++){
        log_info(logger,"%c",puntero_memoria_principal[i]);
    }
}
void print_swap(){
    for (int i=0;i<tamswamp;i++){
        log_info(logger,"%c",swamp[i]);
    }
}