#include "swamp.h"
 


int main(int argc, char ** argv){
   
    init_swamp();

    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
      
    }     

    //prueba_test_20();   //TEST ZONE de SWAMP

    socket_memoria=conectar_a(global_config_swamp->IP, global_config_swamp->PUERTO );
    if (socket_memoria <0){
        perror("debe iniciar primero memoria, no conectado, salgo");
        return EXIT_FAILURE;
    }
    atender_memoria();

    //iniciar_conexion_memoria();
   
    finally_system_swamp();    
}

void init_swamp(void){
    printf("Init process Swamp\n");
    lista_pcb=list_create();
    iniciar_logger();

    cargar_config_file("./cfg/swamp.config");

    inicializar_swamp_files();
}


void finally_system_swamp(){

//	liberar_conexion(conexion);

    finally_config();

    particiones_finally();

    liberar_global_config();
}


void iniciar_conexion_memoria(){

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family=AF_INET;
	direccionServidor.sin_addr.s_addr=inet_addr(global_config_swamp->IP);
	direccionServidor.sin_port=htons( global_config_swamp->PUERTO);

	 cliente=socket(AF_INET,SOCK_STREAM,0);

	int intentos_maximos = 20;
	int index =0;
	while(index<intentos_maximos){
		index++;

		if(connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor))!=0){
			//perror("No se pudo conectar");
			//printf("Reintentando conectar con la Memoria....\n");
            log_info(logger_swamp,"Reintentando conectar con la Memoria....");
			sleep(2);

			if(index== intentos_maximos){  perror("No se pudo conectar"); return;  }
			//return 1;
		}else{
			//printf("conectado a memoria !\n");
            log_info(logger_swamp,"Conectado con Memoria !" );
			break;
		}

	}

    pthread_t thread;   
	pthread_create(&thread,NULL,(void*)(void*)escuchar_mensajes_socket_server, NULL);
    pthread_detach(thread); 

    //crear_hilo_recepcion_memoria();

    //crear_conexion_inicial_memoria();


}

/*
//revisar
int handshake(void){
    char* mensajes[2] = {string_itoa(9), "cliente_config->id_cliente" };
    int socket = send_messages_and_return_socket("swamp",global_config_swamp->IP , string_itoa(global_config_swamp->PUERTO), mensajes, 1);
    if (socket == -1){
        return -1;
    }
    char * mensaje = receive_simple_message(socket);
    if (mensaje == NULL){
        return -1;
    }
    printf("El handshake con el modulo fue correcto\n");
    escuchar_mensajes_socket_desacoplado(socket);   
    return 0;
}


void escuchar_mensajes_socket_desacoplado(int socket){
    
    pthread_t thread;
    t_parameter* parametro = malloc(sizeof(t_parameter));

	parametro->socket = socket;
	parametro->f = handle_client;

	pthread_create(&thread,NULL,(void*)escuchar_mensajes_socket, parametro);
    pthread_detach(thread); 

    //sleep(5);

}

void escuchar_mensajes_socket(t_parameter* parametro){
    escuchar_socket_sin_conexion(&parametro->socket, parametro->f);
}

void handle_client(t_result* result){

    for(int i = 0; i < *result->mensajes->size; i++){
        printf("%s", result->mensajes->mensajes[i]);
    }
    printf("\n");

}
*/

void crear_conexion_inicial_memoria(void){


        
    sleep(2);
    
	enviar_mensaje("swamp", cliente);
    char* mensajes[1] = { string_itoa(9) };
    enviar_mensajes(1,  mensajes,  cliente );
     

    while(1){    //  SI NO ESTA SE CORTA LA CONEXION ?   --MODULO DE ENVIO
        sleep(2);
        if(cliente == -1){ 
            log_info(logger_swamp, "Se corto  la conexion con memoria");
            //printf("se corto  la conexion con memoria\n"); 
             break;}
    }
    
    cliente = -1;
    return;

}

void crear_hilo_recepcion_memoria(void){

 	pthread_t thread;   
	pthread_create(&thread,NULL,(void*)escuchar_mensajes_socket_server, NULL);
    pthread_detach(thread); 

}  

void escuchar_mensajes_socket_server(void){


    char * buffer = malloc(99);

    while(1){
          
    t_mensajes * mensajes =  receive_simple_messages(cliente ); 

        if( mensajes->mensajes[0]  == NULL ){
            log_info(logger_swamp, "Se desconecto Memoria" );
           // printf("se desconecto Memoria\n");
            liberar_conexion(cliente);
            pthread_exit(NULL);
            return;
        }

    //printf("\nrecibi codigo: %s \n", mensajes->mensajes[0]);
 
         handle_mensajes(mensajes);
	}

    free(buffer);
}



/*
//// ----------------------SERIALIZACION----- payloader     OP_CODE || SIZE ||  STREAM   --------------------------     
char* obtener_String(void* buffer,int bytes){     //--------------- MENSAJE SIMPLE -----
	op_code codigo;
	int offset=0;
	memcpy(&codigo,buffer+offset,sizeof(op_code));
	offset+=sizeof(op_code);

    printf("OP CODE: %d \n",  codigo );     //-----------------------

	int size;

	memcpy(&size,buffer+offset,sizeof(int));
	offset+=sizeof(int);

     printf("SIZE: %d \n",  size );    ///-----------------------

	char * stream=malloc(size);
	memcpy(stream,buffer+offset,size);

     printf("STREAM: %s \n",  stream );

	return (char*)stream;
}

*/