#include "memoria.h"

void iniciar_carpincho(int socket_cliente){
	//cargo estructuras para manejar un nuevo proceso carpincho
	t_pcb *pcb=malloc(sizeof(t_pcb));
	pcb->socket=socket_cliente;
	pcb->pid=pid++;
	log_info(logger,"creare un nuevo proceso con pid %i en ram",pcb->pid);
	sem_wait(&sem_swamp);
	enviar_codigo(OP_INICIAR_PROCESO,socketSwamp);
	enviar_codigo(pcb->pid,socketSwamp);
	sem_post(&sem_swamp);
	pcb->tabla_paginas =NULL;
	pcb->alocado =0;
	pcb->clock =0;
	pcb->frames=NULL;
	pcb->inicioVuelta=0;
	pcb->vuelta=0;
	pcb->paginas =0;
	t_contadores *nuevo =malloc (sizeof(t_contadores));
	nuevo->pid = pcb->pid;
	nuevo->count_tlb_hit =0;
	nuevo->count_tlb_miss =0;
	sem_wait(&sem_list);
	list_add(listaContadores,nuevo);
	list_add(listaPCB,pcb);
	sem_post(&sem_list);
	pthread_create(&pcb->hilo,NULL,(void *)atiende_carpincho,pcb);
	pthread_detach(pcb->hilo);
}
void *atiende_carpincho(t_pcb *pcb){
	log_info(logger,"inicio el carpincho con pid %i",pcb->pid);
	enviar_codigo(OK,pcb->socket);
	int codigo;
	while (1){
		codigo=recibir_codigo(pcb->socket);
        switch(codigo){
            case memalloc:
            recibo_memalloc(pcb);
            break;
            case memfree:
        	recibo_memfree(pcb);
        	break;
            case memread:
        	recibo_memread(pcb);
        	break;
            case memwrite:
        	recibo_memwrite(pcb);
        	break;
			case memsuspend:
			recibo_memsuspend(pcb);
			break;
            default:
			recibo_close(pcb);
        	log_warning(logger, "salgo");
			return;
            
        }		
	}
}
void recibo_memalloc(t_pcb *pcb){
	int size=recibir_codigo(pcb->socket);
	log_info(logger,"el carpincho con pid %i envio memalloc son size %i",pcb->pid,size);
	return enviar_codigo(alocar_memoria(size,pcb),pcb->socket);
	
}

void recibo_memread(t_pcb *pcb){
	uint32_t origen=recibir_codigo(pcb->socket);
	int size=recibir_codigo(pcb->socket);
	void *datoLeido=malloc(size);
	log_info(logger,"el carpincho con pid %i envio memread son size %i y puntero a %i",pcb->pid,size,origen);
	int res=leer_memoria(datoLeido,origen,size,pcb);
	if (res<0){
		free(datoLeido);
		enviar_codigo(res,pcb->socket);
		return;
	}
	enviar_codigo_y_datos(res,datoLeido,size,pcb->socket);
	enviar_codigo(res,pcb->socket);
	free(datoLeido);
	return;
}

void recibo_memwrite(t_pcb *pcb){
	uint32_t destino=recibir_codigo(pcb->socket);
	int size=recibir_codigo(pcb->socket);
	void *datoEscribir=recibir_datos(pcb->socket);
	log_info(logger,"el carpincho con pid %i envio memwrite son size %i y puntero a %i",pcb->pid,size,destino);
	enviar_codigo(escribir_memoria(datoEscribir,destino,size,pcb),pcb->socket);
	free(datoEscribir);
	return;
}

void recibo_memfree(t_pcb *pcb){
	int puntero=recibir_codigo(pcb->socket);
	log_info(logger,"el carpincho con pid %i envio memafree al puintero a %i",pcb->pid,puntero);

	return enviar_codigo(liberar_memoria(puntero,pcb),pcb->socket);
}
void recibo_memsuspend(t_pcb *pcb){
	return suspender_proceso(pcb);
}
void recibo_close(t_pcb *pcb){
	log_info(logger,"el carpincho con pid %i finaliza, libero estructuras y memoria",pcb->pid);

	return finalizar_proceso(pcb);
}

int nuevo_proceso_ram(t_pcb *PCB){
	log_info(logger,"creare un nuevo proceso en ram");
//cargo estructuras para manejar un nuevo proceso carpincho
///tengop que reservar lugar si es estatico
	PCB->tabla_paginas =NULL;
	PCB->alocado =0;
	PCB->clock =0;
	t_contadores *nuevo =malloc (sizeof(t_contadores));
	nuevo->pid = PCB->pid;
	nuevo->count_tlb_hit =0;
	nuevo->count_tlb_miss =0;
	list_add(listaContadores,nuevo);
	list_add(listaPCB,PCB);
	return 0;
}

uint32_t alocar_memoria(uint32_t tamanio, t_pcb *PCB){
	return buscar_hueco(tamanio,PCB);
}

int leer_memoria(void *dato,uint32_t inicio,uint32_t tamanio, t_pcb *PCB){
	
	return validar_leer(dato,tamanio,inicio,PCB);
	
}


int escribir_memoria(void *dato,uint32_t inicio,uint32_t tamanio, t_pcb *PCB){
	
	return validar_escribir (dato,tamanio,inicio,PCB);
	
}

int liberar_memoria(uint32_t direccion,t_pcb *PCB){
	return liberar(direccion,PCB);
}



int reservar_frames_swap(uint32_t cant,t_pcb *PCB){
	sem_wait(&sem_swamp);
	enviar_codigo(OP_RESERVAR_PAGINAS,socketSwamp);
	enviar_codigo(PCB->pid,socketSwamp);
	enviar_codigo(cant,socketSwamp);
	if(recibir_codigo(socketSwamp)<0){
		sem_post(&sem_swamp);
		log_info(logger,"swamp no pudo reservar la memoria en este momento");
		return -1;
	}
	sem_post(&sem_swamp);
	log_info(logger,"swamp reservo las paginas");
	return 0;

}


void *traer_pagina(u_int32_t nroPagina,t_pcb *PCB){
	sem_wait(&sem_swamp);
	enviar_codigo(OP_PEDIR_PAGINA,socketSwamp);
	enviar_codigo(PCB->pid,socketSwamp);
	enviar_codigo(nroPagina,socketSwamp);
	//void *dato=recibir_buffer(&memoria_config->TAMANIO_PAGINA, socketSwamp);
	void *dato=malloc(memoria_config->TAMANIO_PAGINA);
	recv(socketSwamp,dato,memoria_config->TAMANIO_PAGINA,0);
	sem_post(&sem_swamp);
	return dato;
	//return traer_pagina_test(nroPagina,PCB);
}

uint32_t enviar_pagina(void *contenido,uint32_t nrpPagina, t_pcb *PCB){
	sem_wait(&sem_swamp);
	enviar_codigo(OP_ESCRIBIR_PAGINA,socketSwamp);
	enviar_codigo(PCB->pid,socketSwamp);
	enviar_codigo_y_datos(nrpPagina,contenido,memoria_config->TAMANIO_PAGINA,socketSwamp);
	//enviar_datos(contenido,memoria_config->TAMANIO_PAGINA,socketSwamp);
	sem_post(&sem_swamp);
	
	return 0;
	//return enviar_pagina_test(contenido,PCB);
}