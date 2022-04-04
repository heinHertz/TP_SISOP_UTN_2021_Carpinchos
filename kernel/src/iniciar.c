#include "kernel.h"

//#include "iniciar.h"
//#include "manejarProcesos.h"
void iniciar(void){
    sem_planificador=malloc(sizeof(sem_t));
    sem_lista_semaforos=malloc(sizeof(sem_t));
    sem_lista_semaforos_bloqueados=malloc(sizeof(sem_t));
    sem_init(sem_planificador,0,1);
    sem_init(sem_lista_semaforos,0,1);
    sem_init(sem_lista_semaforos_bloqueados,0,1);
    contPid=1;
    logger = log_create("./cfg/kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    leer_config();
    if(strcmp(kernel_config->ALGORITMO_PLANIFICACION,"SJF")==0){
        algoritmo=SJF; 
        log_info(logger, "---> Iniciando planificador con algoritmo SJF");
    }
    else {
        algoritmo=HRRN;
        log_info(logger, "---> Iniciando planificador con algoritmo HRRN");
    }
    // Creo las listas
    crear_listas();
    // Inicio los cpus
    iniciar_cpus();
    cargar_dispositivos();
    iniciar_detector_deadlock();
}

void comenzar_recepcion(void){
    int socket_server = iniciar_server(kernel_config->IP_LOCAL,kernel_config->PUERTO_ESCUCHA);
    log_info(logger,"# Se inicia el servidor [%s] en el socket [%i]",kernel_config->IP_LOCAL,socket_server);

    int socket_cliente;
    while(1){
        socket_cliente=recibir_cliente(socket_server);
        if (socket_cliente<0){
            perror("# Algo salio mal, el server no cerró correctamente, reintentar luego");
            close (socket_server);
            return ;
        }
        log_info(logger,"# Se conecto un carpincho");
        iniciar_carpincho(socket_cliente);
        //ejecutar_planificador_largo();
    }
}

void iniciar_carpincho(int socket_cliente){
    log_info(logger, "---> Iniciando Carpincho NEW\n");
    t_pcb *pcb=malloc(sizeof(t_pcb));
    pcb->estado=NEW;
    pcb->socket_carpincho=socket_cliente;
    pcb->lista_semaforos_asignados = list_create();
    sem_wait(sem_planificador);
    ejecutar_planificador_largo(pcb);
    sem_post(sem_planificador);
    return;
}

void iniciar_cpus(void){
    cpus=malloc(sizeof(t_cpu)*kernel_config->GRADO_MULTIPROCESAMIENTO);
    for (int i=0;i<kernel_config->GRADO_MULTIPROCESAMIENTO;i++){
        cpus[i].mi_semaforo=malloc(sizeof(sem_t));
        cpus[i].hilo=malloc(sizeof(pthread_t));
        sem_init(cpus[i].mi_semaforo,0,0);
        cpus[i].estado=LIBRE;
        cpus[i].numero=i;
        //cpus[i].mi_semaforo=&sem_cpus[i];
        cpus[i].pcb=NULL;
        log_info(logger, "---> Iniciando CPU %d\n", i);
        pthread_create(cpus[i].hilo,NULL,(void *)manejo_cpu,&cpus[i]);
    }
}

void cargar_dispositivos(void){
    char *nombre_dispa = strdup(strtok(kernel_config->DISPOSITIVOS_IO,"["));
    char *nombre_dispf = strdup(strtok(nombre_dispa,"]"));
    char *dispn =NULL;
    char *nombre_disp = strtok_r(nombre_dispf,",",&dispn);
    
    char *delayn=NULL;
    char *delaya=strdup(strtok(kernel_config->DURACIONES_IO,"["));
    char *delayf=strdup(strtok(delaya,"]"));
    char *delay=strtok_r(delayf,",",&delayn);

    int i=0;
    t_io *disp;
    
    while(nombre_disp != NULL ) {
        i++;
        disp= malloc(sizeof(t_io));
        disp->nombre= strdup(nombre_disp);
        disp->delay=atoi(delay);
        disp->pcb=NULL;
        disp->lista_espera=list_create();
        disp->hilo=malloc(sizeof(pthread_t));
        disp->mi_semaforo=malloc(sizeof(sem_t));
        disp->sem_lista=malloc(sizeof(sem_t));
        sem_init(disp->mi_semaforo,0,0);
        sem_init(disp->sem_lista,0,1);
        list_add(lista_io,disp);
        pthread_create(disp->hilo,NULL,(void *)manejo_io,disp);
        nombre_disp = strtok_r(NULL,",",&dispn);
        delay=strtok_r(NULL,",",&delayn);
    }
    free(delayf);
    free(delaya);
    //free(delayn);
    free(nombre_dispa);
    free(nombre_dispf);
    //free(dispn);
}


void leer_config(void){
    t_config *config = config_create("./cfg/kernel.config");
    kernel_config = malloc(sizeof(t_kernel_config));
    kernel_config->IP_MEMORIA = strdup(config_get_string_value(config, "IP_MEMORIA"));
    kernel_config->PUERTO_MEMORIA = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
    kernel_config->PUERTO_ESCUCHA = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
    kernel_config->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    kernel_config->ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
    kernel_config->ALFA = atof(config_get_string_value(config, "ALFA"));
    kernel_config->DISPOSITIVOS_IO = strdup(config_get_string_value(config, "DISPOSITIVOS_IO"));
    kernel_config->DURACIONES_IO = strdup(config_get_string_value(config, "DURACIONES_IO"));
    kernel_config->GRADO_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");;
    kernel_config->GRADO_MULTIPROCESAMIENTO = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
    kernel_config->TIEMPO_DEADLOCK  = config_get_int_value(config, "TIEMPO_DEADLOCK");
    kernel_config->IP_LOCAL = strdup(config_get_string_value(config, "IP_LOCAL"));
    config_destroy(config);
}

void crear_listas(void){
    //creo las listas
    lista_io=list_create();
    lista_semaforos=list_create();
    lista_semaforos_bloqueados =list_create();
    lista_blocked=list_create();
    lista_executing=list_create();
    lista_exit=list_create();
    lista_new=list_create();
    lista_ready=list_create();
    lista_suspended_blocked=list_create();
    lista_suspended_ready=list_create();
    lista_deadlock=list_create();
    lista_bloqueados_por_semaforos=list_create();
}