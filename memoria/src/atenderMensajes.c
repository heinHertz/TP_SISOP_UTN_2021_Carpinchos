#include "memoria.h"

void handle_client(t_result* result){
        int tipo_mensaje = atoi(result->mensajes->mensajes[0]);
        switch(tipo_mensaje){
                    case memalloc:
                    handle_memalloc(result);
                    break;
                    case memfree:
        			handle_memfree(result);
        			break;
                    case memread:
        			handle_memread(result);
        			break;
                    case memwrite:
        			handle_memwrite(result);
        			break;
                    case conexion_swamp:
                    handle_conexion_inicial_swamp(result);
                    break;
                    default:
        			log_warning(logger, "Operacion desconocida.");
                    break;
        }
    return;
}

void handle_memalloc(t_result* result){

    char* pid = result->mensajes->mensajes[1]; // char* pid = result->identificador_cliente;
    char* size_solicitado = result->mensajes->mensajes[2];

    char* respuesta[1];
    respuesta[0] = string_itoa(metodo_memalloc(atoi(pid),atoi(size_solicitado)));
    
    send_messages_socket(result->socket, respuesta, 1);
    liberar_conexion(result->socket);
}

void handle_memfree(t_result* result){

    //Asi recibimos los Parametros que necesitamos
    //char* parametro1 = result->mensajes->mensajes[1];
    //char* parametro2 = result->mensajes->mensajes[2];

    char* respuesta[1];
    //respuesta[0] = metodoQueLoHaga;
    respuesta[0]= "OK";

    send_messages_socket(result->socket, respuesta, 1);
    liberar_conexion(result->socket);
}

void handle_memread(t_result* result){

    //Asi recibimos los Parametros que necesitamos
    //char* parametro1 = result->mensajes->mensajes[1];
    //char* parametro2 = result->mensajes->mensajes[2];

    char* respuesta[1];
    //respuesta[0] = metodoQueLoHaga;
    respuesta[0]= "OK";

    send_messages_socket(result->socket, respuesta, 1);
    liberar_conexion(result->socket);
}

void handle_memwrite(t_result* result){

    //Asi recibimos los Parametros que necesitamos
    //char* parametro1 = result->mensajes->mensajes[1];
    //char* parametro2 = result->mensajes->mensajes[2];

    char* respuesta[1];
    //respuesta[0] = metodoQueLoHaga;
    respuesta[0]= "OK";

    send_messages_socket(result->socket, respuesta, 1);
    liberar_conexion(result->socket);
}


//------------------------------------------------------ MENSAJES CON SWAMP --------------------------------------------------------//

void handle_conexion_inicial_swamp(int socket){
    //cliente_swamp=-1;  //necesito que este antes de iniciar el server


    pthread_mutex_init(&MX_SEND, NULL);
    pthread_mutex_init(&MX_RECV, NULL);

    /*
    printf("Estoy conectado con el Swamp \n");
    printf("RESPUESTA 0 : %s \n", result->mensajes->mensajes[0]);
    printf("RESPUESTA 1 : %s \n", result->mensajes->mensajes[1]);
    */
    char* respuesta[1];

    respuesta[0]= "0";
    respuesta[1]="RECIBI OK";

   // send_messages_socket(result->socket, respuesta, 1);

    cliente_swamp = socket;

    escuchar_mensajes_socket_swamp();

   // liberar_conexion(result->socket);    
}


//  -------------------RECEPCI0N DESDE SWAMP-------------------------//
void escuchar_mensajes_socket_swamp(void ){     

char * buffer = malloc(99);

    enviar_swamp_asignacion_memoria();


	while(1){  ///SIN EL WHILE SE CORTA :(
   
    sleep(2);
    }

}

//  innecesario?
 //     payloader ----> podria depender del Mensaje[0]: COD OP  -- Mensaje[1]: PID  -- Mensaje[2]: *DATA*     
void * handler_respuestas_swamp(t_mensajes * mensajes ){

    int codigo_op = atoi( mensajes->mensajes[0] );

    switch (codigo_op)
    {
        case OP_MENSAJE_SIMPLE:   
        printf("recibi op mensaje simple\n");
        printf("enviando respuesta:\n");
        char* resp[2] = { string_itoa(0) , "RESPUESTA"};
        enviar_mensajes(2,  resp,  cliente_swamp );

        break;
        case OP_MENSAJES_SIMPLES:
        printf("recibi op mensajes simples\n");
        break;
        case OP_FINALIZAR_PROCESO:
        printf("recibi op finalizar proceso\n");
        break;
    //  case OP_RESERVAR_PAGINAS:
    //  printf("recibi op reservar paginas\n");
    //  break;
        case OP_ESCRIBIR_PAGINA:
         procesar_respuesta_pedir_pagina(mensajes);
        break;
        case OP_GUARDAR_PAGINA:        
        break;
        case OP_PEDIR_PAGINA:
        procesar_respuesta_pedir_pagina(mensajes);
        break;
        case OP_TIPO_ASIGNACION_MEMORIA:
        printf("recibi op mensaje simple\n");
        printf("enviando respuesta:\n");
        char* respuestas[2] = { string_itoa(0) , memoria_config->TIPO_ASIGNACION };
        enviar_mensajes(2,  respuestas,  cliente_swamp );
        break;    


    default:
        printf("Error. codigo_op invalido: %d \n", codigo_op);

        break;
    }

}
// innecesario?
void * procesar_respuesta_pedir_pagina(t_mensajes * mensajes){ 
 printf("r- ------------\n");
    printf("recibi : %s   pro resp peri pag \n", mensajes->mensajes[1] );
    printf("recibi : %s   pro resp peri pag \n", mensajes->mensajes[2] );
  
     if(mensajes->mensajes[1] == "OK"){
        return mensajes->mensajes[2];
    }else{
        return NULL;
    }
}




/* 
int reservar_paginas(cantidad, pid) -> return OK + lista de # de paginas reservadas /NoOK si no hay lugar
int liberar_paginas (numero, pid) -> return OK/nada
int enviar_pagina(* void, tamanio, pid) ->return # de pagina donde se guardó
int pedir_pagina(numero, pid) -> return contenido de la pagina
*/

void  enviar_swamp_asignacion_memoria(void){  //-> return OK + lista de # de paginas reservadas /NoOK si no hay lugar

    log_info(logger,"ENVIANDO A SWAMP ASIGNACION MEMORIA: %s \n",memoria_config->TIPO_ASIGNACION); 
    char* respuesta[3];

    respuesta[0]= string_itoa(OP_TIPO_ASIGNACION_MEMORIA);
    respuesta[1]=  memoria_config->TIPO_ASIGNACION;
                
    pthread_mutex_lock(&MX_SEND);
    send_messages_socket(cliente_swamp, respuesta, 2 );
    

    t_mensajes* mensajes = NULL;  
    mensajes = receive_simple_messages(cliente_swamp);
    pthread_mutex_unlock(&MX_SEND);

    log_info(logger,"Recibi respuesta de SWAMP: %s  \n",mensajes->mensajes[1]); 

    return; //  mensajes->mensajes[1]; //handler_respuestas_swamp(mensajes);
} 

void finalizar_conexion_swamp(){

    if(cliente_swamp != -1){
        printf("ENVIANDO A SWAMP AVISO DE DESCONEXION\n");
        char* respuesta[3];
        respuesta[0]= string_itoa(OP_FINALIZAR_CONEXION_SWAMP);
        send_messages_socket(cliente_swamp, respuesta, 1 );
        liberar_conexion(cliente_swamp);   
        return;
    }   
}

///------   FUNCIONES -------- //// 
//  ANTES DE MANDAR LAS DEMAS FUNCIONES SI O SI MANDAR ESTA INICIAR PROCESO A SWAMP, PORQUE ES PARA INICIALIZAR EL PROCESO EN SWAMP
int  iniciar_proceso_swamp(int pid){   

    log_info(logger,"ENVIANDO A SWAMP INICIAR PROCESO PID %d\n", pid);
    char* respuesta[3];

    respuesta[0]= string_itoa(OP_INICIAR_PROCESO);
    respuesta[1]= string_itoa(pid);
             
    pthread_mutex_lock(&MX_SEND);
    send_messages_socket(cliente_swamp, respuesta, 2 );
    

    t_mensajes* mensajes = NULL;  
    mensajes = receive_simple_messages(cliente_swamp);
    pthread_mutex_unlock(&MX_SEND);
  
    log_info(logger,"Respuesta de SWAMP: %s\n", mensajes->mensajes[1]);
    int marco_respuesta;

    char *string;
    string = string_duplicate( mensajes->mensajes[1] );
    string_trim(&string);

    if ( strcmp(string, "OK") == 0 ) {    
    marco_respuesta = 0;
    }else{
        marco_respuesta = -1;
    }

    free(string);

    return  marco_respuesta; //handler_respuestas_swamp(mensajes);
} 

// pedido de lectura de página realizado la memoria, este módulo devolverá el contenido de esta página.
// ok = chorro de bytes tamanio=pagina    FAIL = NULL
void * pedir_pagina_swamp(int numero_pagina, int pid){  // pide una pagina a la ver ES ESQUIVALENTE A LEER PAGINA en SWAP

    log_info(logger,"Enviando a SWAMP pedido de paginas. Proceso: %d, Numero de pagina: %d \n",pid,numero_pagina);
    char* respuesta[3];

    respuesta[0]= string_itoa(OP_PEDIR_PAGINA);
    respuesta[1]= string_itoa(pid);  //numero de pagina
    respuesta[2]= string_itoa(numero_pagina);
             
    pthread_mutex_lock(&MX_SEND);
    send_messages_socket(cliente_swamp, respuesta, 3 );
    

    t_mensajes* mensajes = NULL;  
    mensajes = receive_simple_messages(cliente_swamp);
    pthread_mutex_unlock(&MX_SEND);
  
    log_info(logger,"Recibi respuesta de SWAMP: %s  \n",mensajes->mensajes[1]); 
    char * buffer = malloc(memoria_config->TAMANIO_PAGINA);
   

    return  mensajes->mensajes[2];
} 


int  guardar_pagina_swamp(int pid, void * buffer){     //tamanio de buffer implicitamente esta en el archivo de config

    log_info(logger,"Enviando a SWAMP buffer para guardar pagina del prorceso: %d\n", pid);
    char* respuesta[3];

    respuesta[0]= string_itoa(OP_GUARDAR_PAGINA);
    respuesta[1]= string_itoa(pid);
    respuesta[2] = buffer;
             
    pthread_mutex_lock(&MX_SEND);
    send_messages_socket(cliente_swamp, respuesta, 3 );
    

    t_mensajes* mensajes = NULL;  
    mensajes = receive_simple_messages(cliente_swamp);
    pthread_mutex_unlock(&MX_SEND);
  
    log_info(logger,"Recibi respuesta de SWAMP: %s  \n",mensajes->mensajes[1]); 
    char *string;
    string = string_duplicate( mensajes->mensajes[1] );
    string_trim(&string);

    int marco;     

     if ( strcmp(string, "OK") == 0 ) { 
     marco = atoi(mensajes->mensajes[2]);
    }else{
        marco = -1;
    }

    free(string);
    return  marco; //handler_respuestas_swamp(mensajes);
} 

// PARA QUE EL PID? PARA ALMACENAR EN LA PARTICION EN LA QUE SE ENCUENTRE ESE PID Y EL NUMERO DE PAGINA ES "FISICO", DESDE LA POSICION N DE PARTICION
void * escribir_pagina_swamp(int pid, int numero_pagina, void * buffer){     //tamanio de buffer implicitamente esta en el archivo de config OK


    log_info(logger,"Enviando a SWAMP buffer escribir pagina: %d ,del proceso: %d\n",numero_pagina ,pid);
    char* respuesta[4];

    respuesta[0]= string_itoa(OP_ESCRIBIR_PAGINA);
    respuesta[1]= string_itoa(pid);
    respuesta[2]= string_itoa(numero_pagina);
    respuesta[3] = buffer;
             
    pthread_mutex_lock(&MX_SEND);
    send_messages_socket(cliente_swamp, respuesta, 4 );
    

    t_mensajes* mensajes = NULL;  
    mensajes = receive_simple_messages(cliente_swamp);
    pthread_mutex_unlock(&MX_SEND);
  
    log_info(logger,"Recibi respuesta de SWAMP: %s  \n",mensajes->mensajes[1]);  

    return  mensajes->mensajes[1]; //handler_respuestas_swamp(mensajes);
} 


void * finalizar_proceso_swamp(int pid){  //OK

    printf("ENVIANDO A SWAMP FINALIZAR PROCESO PID %d\n", pid);
    char* respuesta[3];

    respuesta[0]= string_itoa(OP_FINALIZAR_PROCESO);
    respuesta[1]= string_itoa(pid);
             
    pthread_mutex_lock(&MX_SEND);
    send_messages_socket(cliente_swamp, respuesta, 2 );
    

    t_mensajes* mensajes = NULL;  
    mensajes = receive_simple_messages(cliente_swamp);
    pthread_mutex_unlock(&MX_SEND);
  
    //falta  logger_info 

    return  mensajes->mensajes[1]; //handler_respuestas_swamp(mensajes);
} 