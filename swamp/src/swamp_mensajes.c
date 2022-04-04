#include "swamp_mensajes.h"

void atender_memoria(void){
    //intercambio mensajes iniciales con memoria
    //recibo el metodo de asignacion (fijo o dinamico)
    TIPO_ASIGNACION_MEMORIA=recibir_codigo(socket_memoria);
    //envio la cantidad de marcos por proceso a memoria
    enviar_codigo(global_config_swamp->MARCOS_POR_PROCESO,socket_memoria);
    int codigo_operacion;
    while(1){
        codigo_operacion =recibir_codigo(socket_memoria);
    
        switch (codigo_operacion){
            case OP_INICIAR_PROCESO:
            iniciar_carpincho();
            break;
            case OP_PEDIR_PAGINA:
            usleep(global_config_swamp->RETARDO_SWAP*1000);
            pedir_pagina();
            
            break;
            case OP_ESCRIBIR_PAGINA:
            usleep(global_config_swamp->RETARDO_SWAP*1000);
            escribir_pagina();
            
            break;
            case OP_RESERVAR_PAGINAS:
            reservar_paginas();
            break;
            case OP_FINALIZAR_PROCESO:
            finalizar_carpincho();
            break;     
            default:
            finalizar_conexion_con_memoria();
            return;
        }  
    }
}
void iniciar_carpincho(void){
    int pid=recibir_codigo(socket_memoria);
    inicializar_proceso_en_el_sistema_particiones(pid);
    return;
}
void pedir_pagina(void){
    int pid=recibir_codigo(socket_memoria);
    int pagina=recibir_codigo(socket_memoria);
    log_info(logger_swamp,"el carpincho %i pide la pagina %i",pid,pagina);
    char *stream = leer_marco_proceso(pid,pagina);
    send(socket_memoria,stream,global_config_swamp->TAMANIO_PAGINA,0);
    free(stream);
}
void escribir_pagina(void){
    int pid=recibir_codigo(socket_memoria);
    int pagina=recibir_codigo(socket_memoria);
    log_info(logger_swamp,"el carpincho %i envia la pagina %i",pid,pagina);
    void *buffer=recibir_datos(socket_memoria);
    escribir_pagina_proceso(pid,pagina,buffer);
    free(buffer);
}
void reservar_paginas(void){
    int pid=recibir_codigo(socket_memoria);
    int cant=recibir_codigo(socket_memoria);
    enviar_codigo(reservo_paginas(pid,cant),socket_memoria);
    return;
}
void finalizar_carpincho(void){
    int pid=recibir_codigo(socket_memoria);
    log_info(logger_swamp,"el carpincho %i finaliza",pid);
    finalizar_proceso(pid);
}
void finalizar_conexion_con_memoria(void){
    //libero los pcbs
    t_pcb *destroy;
    log_info(logger_swamp,"la memoria finalizo, salgo");
    for(int i=1;i<list_size(lista_pcb);i++){
        destroy =list_remove(lista_pcb,i);
        free(destroy->tabla_de_paginas);
        free(destroy);
    }
    return;
}




void handle_mensajes(t_mensajes * mensajes){

     
    int codigo_operacion = atoi( mensajes->mensajes[0] );

    switch (codigo_operacion)
    {
        case OP_MENSAJE_SIMPLE:   
        printf("RECIBI cod op  %s MENSAJE SIMPLE : %s  \n", mensajes->mensajes[0], mensajes->mensajes[1] ); //es de test
        break;
        case OP_MENSAJES_SIMPLES:  //es de test
        //log_info(logger_swamp, "RECIBI cod op  MENSAJES ");
        printf("RECIBI cod op  MENSAJES \n" );
        break;
        case OP_TIPO_ASIGNACION_MEMORIA:         
        handle_asignacion(mensajes);
        break;
        case OP_PEDIR_PAGINA:
         handle_pedir_pagina(mensajes);
        break;
        case OP_INICIAR_PROCESO:
        handle_iniciar_proceso(mensajes);
        break;
        case OP_GUARDAR_PAGINA:
        handle_guardar_pagina(mensajes);         
        break;
        case OP_ESCRIBIR_PAGINA:
        handle_escribir_pagina(mensajes);
        break;
//      case OP_RESERVAR_PAGINAS:
//      handle_reservar_paginas(mensajes);
//      break;
        case OP_FINALIZAR_PROCESO:
        handle_finalizar_proceso(mensajes);
        break;  
        case OP_FINALIZAR_CONEXION_SWAMP:
        handle_finalizar_conexion_con_memoria(mensajes);
        break;     
    default:
        printf("Error. TipoMensaje invalido: %d", codigo_operacion);
        break;
    }         
} 


 void handle_mensaje_simples(t_result* result){

	 printf("Recibi el mensaje: %s \n" , result->mensajes->mensajes[0] );   
	 printf("Recibi el mensaje: %s \n" , result->mensajes->mensajes[1] );    
    char* respuesta[1];
    char* string = "TeEnvioRespuesta";
    
    if (TRUE){    
    respuesta[0] = malloc(strlen(string) );
    strcpy(respuesta[0], string);        
    } else {
        respuesta[0] = "FAIL";
    }
       
    enviar_mensaje(respuesta[0],result->socket);  
  // send_messages_socket(result->socket, respuesta, 1);
   // liberar_conexion(result->socket);
    log_info( logger_swamp, "Mensaje Simple Recibido" );    
    free(respuesta[0]);
 }


void handle_handshake_memoria(t_result* result){
    
    char* respuesta[1];
    char* string = "Mensaje:TeEnvioMensajeTexto";
    if (TRUE){ //HAcer lo que tenga que hacer
   
    respuesta[0] = malloc(strlen(string) );
    strcpy(respuesta[0], string);
     //   respuesta[0] ="OKKKKK";//malloc(strlen(string));
        
    } else {
        respuesta[0] = "FAIL";
    }
       
    send_messages_socket(result->socket, respuesta, 1);
    liberar_conexion(result->socket);

    free(respuesta[0]);
}


void handle_pedir_pagina(t_mensajes * mensajes){

    log_info(logger_swamp, "Recibi el mensaje PEDIR PID: %s , PAGINA: %s \n", mensajes->mensajes[1], mensajes->mensajes[2] );
   // printf("Recibi el mensaje PEDIR PID: %s , PAGINA: %s \n", mensajes->mensajes[1], mensajes->mensajes[2] );

    char* respuesta[10];

    int pid = atoi( mensajes->mensajes[1] );
    int numero_pagina = atoi( mensajes->mensajes[2] );

   // printf(" pid:  %d, pagima: %d ", pid, numero_pagina );


    //------------    
    char * stream = leer_marco_proceso( pid,  numero_pagina );
    //----------

    if (TRUE){ //HAcer lo que tenga que hacer

        respuesta[0] = string_itoa(OP_PEDIR_PAGINA); 
        respuesta[1] = "OK";
        respuesta[2] = stream;


    } else {
        respuesta[0] = string_itoa(OP_PEDIR_PAGINA);
        respuesta[1] = "FAIL";
        respuesta[2] = NULL;
    }
    
    log_info(logger_swamp, "RESPONDIENDO A MEMORIA PEDIDO DE PAGINA con retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);

    usleep(global_config_swamp->RETARDO_SWAP);

    send_messages_socket(cliente, respuesta, 3);
    //liberar_conexion(result->socket);
}


void handle_iniciar_proceso(t_mensajes * mensajes){

    log_info(logger_swamp,"Recibi el mensaje INICIAR PROCESO: %s , PID: %s \n", mensajes->mensajes[0], mensajes->mensajes[1] );

   // printf("Recibi el mensaje INICIAR PROCESO: %s , PID: %s \n", mensajes->mensajes[0], mensajes->mensajes[1] );

    char* respuesta[10];
   
    int pid = atoi(  mensajes->mensajes[1] );
   
    //---------FUNCION-----
    int resultado = inicializar_proceso_en_el_sistema_particiones(pid);
    //----------

    if (resultado == 0){ 

        respuesta[0] = string_itoa(OP_INICIAR_PROCESO); 
        respuesta[1] = "OK";

    } else {
        respuesta[0] = string_itoa(OP_INICIAR_PROCESO);
        respuesta[1] = "FAIL";
    }

    //LOGGER ACA

    log_info(logger_swamp, "RESPONDIENDO A INICIALIZAR PROCESO con retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);
   
    usleep(global_config_swamp->RETARDO_SWAP);

    send_messages_socket(cliente, respuesta, 2);
    //liberar_conexion(result->socket);
}


void handle_finalizar_proceso(t_mensajes * mensajes){

    log_info(logger_swamp, "Recibi el mensaje FINALIZAR PROCESO: %s , PID: %s \n", mensajes->mensajes[0], mensajes->mensajes[1] );
  //  printf("Recibi el mensaje FINALIZAR PROCESO: %s , PID: %s \n", mensajes->mensajes[0], mensajes->mensajes[1] );

    char* respuesta[10];

    int pid = atoi(   mensajes->mensajes[1] );

    //------------
    finalizar_proceso(pid);
    //----------

    if (TRUE){ //HAcer lo que tenga que hacer

        respuesta[0] = string_itoa(OP_FINALIZAR_PROCESO); 
        respuesta[1] = "OK";

    } else {
        respuesta[0] = string_itoa(OP_FINALIZAR_PROCESO);
        respuesta[1] = "FAIL";
    }

    //LOGGER ACA
    log_info(logger_swamp, "FINALIZANDO PROCESO: %d  con retardo de:  %d \n", pid, global_config_swamp->RETARDO_SWAP);

  //  printf("respondiendo con finalizar proceso retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);
    usleep(global_config_swamp->RETARDO_SWAP);

    send_messages_socket(cliente, respuesta, 2);
    //liberar_conexion(result->socket);
}

void handle_guardar_pagina(t_mensajes * mensajes){

    log_info(logger_swamp,"Recibi el mensaje GUARDAR PAGINA: cod op %s , PID: %s , buffer %s \n", mensajes->mensajes[0], mensajes->mensajes[1],  mensajes->mensajes[2] );

  //  printf("Recibi el mensaje GUARDAR PAGINA: cod op %s , PID: %s , buffer %s \n", mensajes->mensajes[0], mensajes->mensajes[1],  mensajes->mensajes[2] );

    char* respuesta[10];

    int pid = atoi( mensajes->mensajes[1] );
    
    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memcpy(buffer, mensajes->mensajes[2] , global_config_swamp->TAMANIO_PAGINA );

    //------------
    int marco_respuesta =  guardar_marco_en_swamp(pid, buffer);
    //----------

    if (TRUE){ //HAcer lo que tenga que hacer

        respuesta[0] = string_itoa(OP_GUARDAR_PAGINA); 
        respuesta[1] = "OK";
        respuesta[2] = string_itoa( marco_respuesta );

    } else {
        respuesta[0] = string_itoa(OP_GUARDAR_PAGINA);
        respuesta[1] = "FAIL";
        respuesta[2] = "FAIL";
    }

    free(buffer);


    log_info(logger_swamp, "RESPONDIENDO A GUARDAR PAGINA con retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);

  //  printf("respondiendo guardar pagina con retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);
    usleep(global_config_swamp->RETARDO_SWAP);

    send_messages_socket(cliente, respuesta, 3);
    //liberar_conexion(result->socket);
}


void handle_escribir_pagina(t_mensajes * mensajes){


    log_info( logger_swamp, "Recibi el mensaje %s ESCRIBIR PAGINA: PID: %s, numero de pagina %s   \n", mensajes->mensajes[0], mensajes->mensajes[1] , mensajes->mensajes[2] );

//    printf("Recibi el mensaje %s ESCRIBIR PAGINA: PID: %s, numero de pagina %s   \n", mensajes->mensajes[0], mensajes->mensajes[1] , mensajes->mensajes[2] );

    char* respuesta[10];

    int pid = atoi( mensajes->mensajes[1] );
    int numero_pagina = atoi( mensajes->mensajes[2] );
    
    char * buffer = malloc( global_config_swamp->TAMANIO_PAGINA);
    memcpy(buffer, mensajes->mensajes[3], global_config_swamp->TAMANIO_PAGINA );

    //------------
    escribir_pagina_proceso(pid, numero_pagina, buffer  );
    //----------

    if (TRUE){ //HAcer lo que tenga que hacer

        respuesta[0] = string_itoa(OP_ESCRIBIR_PAGINA); 
        respuesta[1] = "OK";

    } else {
        respuesta[0] = string_itoa(OP_ESCRIBIR_PAGINA);
        respuesta[1] = "FAIL";
    }

    log_info(logger_swamp, "RESPONDIENDO A MEMORIA ESCRIBIR DE PAGINA con retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);

    usleep(global_config_swamp->RETARDO_SWAP);

    send_messages_socket(cliente, respuesta, 2);
    //liberar_conexion(result->socket);
}


void handle_asignacion(t_mensajes * mensajes ){


    log_info(logger_swamp, "RECIBI cod op: %s , TIPO_ASIGNACION_MEMORIA: %s \n",  mensajes->mensajes[0], mensajes->mensajes[1] );
	
    //printf("RECIBI cod op: %s , TIPO_ASIGNACION_MEMORIA: %s \n",  mensajes->mensajes[0], mensajes->mensajes[1] );
	
    char* respuesta[2];
    char *string;

    string = string_duplicate( mensajes->mensajes[1] );
    string_trim(&string);

    if ( strcmp(string, "FIJA") == 0 ) 
    {
        printf("Establecido asignacion fija\n");
        TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;
    }
    if ( strcmp(string, "DINAMICA") == 0  )
    {   printf("Establecido asignacion dinamica\n");
        TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;
    }


    respuesta[0] = string_itoa( TIPO_ASIGNACION_MEMORIA );
    respuesta[1] = "OK";
	
    free(string);    
    
    log_info(logger_swamp, "RESPONDIENDO A MEMORIA PEDIDO DE PAGINA con retardo de:  %d \n", global_config_swamp->RETARDO_SWAP);

    usleep(global_config_swamp->RETARDO_SWAP);

    send_messages_socket(cliente, respuesta, 2);

}


void handle_finalizar_conexion_con_memoria(t_mensajes * mensajes){
    

    log_info(logger_swamp, "FINALIZANDO CONECION CON MEMORIA \n" );
    log_info(logger_swamp, "Iniciando  desconexion con Memoria");
   // printf("Iniciando  desconexion con Memoria\n");
    liberar_conexion(cliente);
    cliente = -1;
    pthread_exit(NULL);
    return;
}


/* 
void handle_reservar_paginas(t_mensajes * mensajes){

    printf("RECIBI cod op: %s   RESERVAR_PAGINAS  %s, de PROCESO PID: %s \n", mensajes->mensajes[0],  mensajes->mensajes[1], mensajes->mensajes[2] );

    char* respuesta[10];

    if (TRUE){ //HAcer lo que tenga que hacer
        respuesta[0] = "RESPUESTA RESERVAR PAGINAS OK";
    } else {
        respuesta[0] = "FAIL";
    }

    send_messages_socket(cliente, respuesta, 1);
   // liberar_conexion(cliente);

}
*/