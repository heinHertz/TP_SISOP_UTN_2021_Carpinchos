#include "memoria.h"

int main(int argc, char ** argv){
	signal(SIGINT,sig_handler);
	signal(SIGUSR1,sig_handler);
    signal(SIGUSR2,sig_handler);

        memoria_init(&memoria_config);

        iniciar_ram();
        //prueba();

        //sleep(4);
        
        //tests_mensajes_swamp(); //Levantar SWAMP antes de los 20 segundos
    socket_server = iniciar_server(memoria_config->IP,memoria_config->PUERTO);
    log_info(logger,"inicie el server en puerto de escucha %i",socket_server);
    int socket_cliente;
    //la primer conexion es de swamp
    log_info(logger,"espero a swamp");
    socket_cliente=recibir_cliente(socket_server);
    if (socket_cliente<0){
        perror("algo salio mal, el server se cerró mal, reintentar en un minuto");
        close (socket_server);
        return 0;
    }
    //primero deberia recibir a swamp e inicializar los datos
    socketSwamp=socket_cliente;
    enviar_codigo(metodo_asignacion,socketSwamp);
    paginasMaximas= recibir_codigo(socketSwamp);
    log_info(logger,"se conecto swamp\nespero un carpincho");
    while(1){//aca recibo nuevos carpinchos
        socket_cliente=recibir_cliente(socket_server);
        if (socket_cliente<0){
            printf("finalice el server");
            close (socket_server);
            return 0;
        }

        log_info(logger,"un carpincho se conecto");
        iniciar_carpincho(socket_cliente);
    }
        
        //iniciar_servidor(memoria_config->IP,memoria_config->PUERTO, handle_client);
      
     
   
    return 0;  
 }

void finalizar_carpinchos(void){
    t_pcb *pcb;
    for (int i=0;i<list_size(listaPCB);i++){
        
        pcb =list_remove(listaPCB,0);
        finalizar_proceso(pcb);
        //close(pcb->socket);
        pthread_kill(pcb->hilo,-9);
        //free(pcb->tabla_paginas);
        //free(pcb->frames);
        //close(pcb->socket);
        //free(pcb);
    }
    list_destroy(listaPCB);
}


	void sig_handler(int signum){
		switch(signum){
			case SIGINT:{
                sem_wait(&sem_list);
                t_contadores *cont;
                log_info(logger,"Recibi interrupcion para imprimir metricas y salir");
                close (socket_server);
               
                finalizar_carpinchos();
               
               
                printf("Cantidad de TLB Hit totales: %i\n",tothit);
                for (int i=0;i<list_size(listaContadores);i++){
                    cont =list_get(listaContadores,i);
                    printf("total hits del carpincho %i: %i\n",cont->pid,cont->count_tlb_hit);
                }
                printf("Cantidad de TLB Miss totales: %i\n",totmiss);
                for (int i=0;i<list_size(listaContadores);i++){
                    cont =list_remove(listaContadores,0);
                    printf("total miss del carpincho %i: %i\n",cont->pid,cont->count_tlb_miss);
                    free(cont);
                }
                list_destroy(listaContadores);
                close(socketSwamp);
                close(socket_server);
                memoria_finally(memoria_config, logger);
				
				
				//TODO: imprimirMetricas();
			
				break;
			}
			case SIGUSR1:{
				log_info(logger,"Recibi interrupcion para generar dump de la TLB");
				
				generar_dump();
				
			}
            case SIGUSR2:{
				log_info(logger,"Recibi interrupcion para generar vaciar la TLB");
				sem_wait(&sem_tlb);
                for(int i=0;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
	            	TLB[i].cont=0;
	            	TLB[i].frame=0;
	            	TLB[i].pid=0;
	            	TLB[i].pagina=0;
	            }
                sem_post(&sem_tlb);
				//TODO: generarDUMP();
				
			}
		}
	}

void memoria_init(t_memoria_config** memoria_config){
    logger = log_create("./cfg/memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
    log_info(logger, "Soy el modulo memoria!");
    logram = log_create("./cfg/logram.log", "RAM", true, LOG_LEVEL_INFO);

    *memoria_config = memoria_config_loader("./cfg/memoria.config");
}

t_memoria_config* memoria_config_loader(char* path_config_file) {
    t_config* config = config_create(path_config_file);
    t_memoria_config* memoria_config = malloc(sizeof(t_memoria_config));

    memoria_config_parser(config, memoria_config);
    config_destroy(config);
    return memoria_config;
}

void memoria_config_parser(t_config* config, t_memoria_config* memoria_config) {
    memoria_config->IP = strdup(config_get_string_value(config, "IP"));
    memoria_config->PUERTO = strdup(config_get_string_value(config, "PUERTO"));
    memoria_config->TAMANIO = config_get_int_value(config, "TAMANIO");
    memoria_config->TAMANIO_PAGINA = config_get_int_value(config, "TAMANIO_PAGINA");
    memoria_config->TIPO_ASIGNACION = strdup(config_get_string_value(config, "TIPO_ASIGNACION"));
    memoria_config->ALGORITMO_REEMPLAZO_MMU = strdup(config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU"));
    memoria_config->CANTIDAD_ENTRADAS_TLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    memoria_config->ALGORITMO_REEMPLAZO_TLB = strdup(config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB"));
    memoria_config->RETARDO_ACIERTO_TLB = config_get_int_value(config, "RETARDO_ACIERTO_TLB");
    memoria_config->RETARDO_FALLO_TLB = config_get_int_value(config, "RETARDO_FALLO_TLB");
    memoria_config->MARCOS_POR_CARPINCHO  = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
    memoria_config->PATH_DUMP_TLB = strdup(config_get_string_value(config, "PATH_DUMP_TLB"));
}

void memoria_finally(t_memoria_config* memoria_config, t_log* logger) {
    free(mapaDeBits);
    free(puntero_memoria_principal);
   memoria_destroy(memoria_config);
    log_destroy(logger);
     log_destroy(logram);
     free(TLB);
}

void memoria_destroy(t_memoria_config* memoria_config) {
    free(memoria_config->IP);
    free(memoria_config->PUERTO);
    free(memoria_config->TIPO_ASIGNACION);
    free(memoria_config->ALGORITMO_REEMPLAZO_MMU);
    free(memoria_config->ALGORITMO_REEMPLAZO_TLB);
    free(memoria_config->PATH_DUMP_TLB);
    free(memoria_config);

}

