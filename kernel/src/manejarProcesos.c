#include "kernel.h"
#include "codigos_operacion.h"
#include "server.h"

void *manejo_cpu(t_cpu *cpu){
    log_info(logger,"# Se inicia el hilo de CPU [%i]",cpu->numero);
    int codComando;
    cpu->resp=OK;
    while(1){
        sem_wait(cpu->mi_semaforo);
        log_info(logger,"# El CPU [%i] espera algo del carpincho [%i]",cpu->numero,cpu->pcb->pid);
        enviar_codigo(cpu->resp,cpu->pcb->socket_carpincho);
        codComando=recibir_codigo(cpu->pcb->socket_carpincho);
        switch (codComando){
            //funciones de memoria
            case memalloc:
                manejo_memalloc(cpu);
                sem_post(cpu->mi_semaforo);
                break;
            case memfree:
                manejo_memfree(cpu);
                sem_post(cpu->mi_semaforo);
                break;
            case memread:
                manejo_memread(cpu);
                sem_post(cpu->mi_semaforo);
                break;
            case memwrite:
                manejo_memwrite(cpu);
                sem_post(cpu->mi_semaforo);
                break;
            //funciones de semaforos
            case SEMINIT://no es bloqueante
                manejo_seminit(cpu);
                sem_post(cpu->mi_semaforo);
                break;
            case SEMWAIT://puede ser bloqueante
                manejo_semwait(cpu);
                break;
            case SEMPOST:
                manejo_sempost(cpu);//no es bloqueante
                sem_post(cpu->mi_semaforo);
                break;
            case SEMDESTROY://no es bloqueante
                manejo_semdestroy(cpu);
                sem_post(cpu->mi_semaforo);
                break;
            //funciones de io
            case MATEIO://es bloqueante
                manejo_mateio(cpu);            
                break;
            case MATECLOSE://es bloqueante del cpu
                manejo_mateclose(cpu);
                break;
            default:
                log_info(logger,"el carpincho %i finaliza anormalmente",cpu->pcb->pid);
                manejo_mateclose(cpu);
                break;
        }
    }
    return NULL;
}


void manejo_memalloc(t_cpu *cpu){
    if(cpu->pcb->socket_memoria<0){return;}
    //recibo del carpincho
    int size=recibir_codigo(cpu->pcb->socket_carpincho);
    log_info(logger, ">> Se recibe un memalloc del carpincho %d\n", cpu->pcb->pid);
    //envio a memoria
    enviar_codigo(memalloc,cpu->pcb->socket_memoria);
    enviar_codigo(size,cpu->pcb->socket_memoria);
    cpu->resp=recibir_codigo(cpu->pcb->socket_memoria);
    return;
}

void manejo_memfree(t_cpu *cpu){
    if(cpu->pcb->socket_memoria<0){return;}
    //recibo del carpincho
    int addr=recibir_codigo(cpu->pcb->socket_carpincho);
    log_info(logger, ">> Se recibe un memfree del carpincho %d\n", cpu->pcb->pid);
    //envio a memoria
    enviar_codigo(memfree,cpu->pcb->socket_memoria);
    enviar_codigo(addr,cpu->pcb->socket_memoria);
    cpu->resp=recibir_codigo(cpu->pcb->socket_memoria);
    return;
}

void manejo_memread(t_cpu *cpu){
    if(cpu->pcb->socket_memoria<0){return;}
    //recibo del carpincho
    int origin=recibir_codigo(cpu->pcb->socket_carpincho);
    int size=recibir_codigo(cpu->pcb->socket_carpincho);
    log_info(logger, ">> Se recibe un memread del carpincho %d\n", cpu->pcb->pid);
    //envio a memoria
    
    enviar_codigo(memread,cpu->pcb->socket_memoria);
    enviar_codigo(origin,cpu->pcb->socket_memoria);
    enviar_codigo(size,cpu->pcb->socket_memoria);
    
    int resp=recibir_codigo(cpu->pcb->socket_memoria);
    if (resp>=0){
        void *data=recibir_buffer(&size,cpu->pcb->socket_memoria);
        enviar_codigo_y_datos(1,data,size,cpu->pcb->socket_carpincho);
        recibir_codigo(cpu->pcb->socket_memoria);
        free(data);
    }
    cpu->resp=resp;
    //return resp;
}

void manejo_memwrite(t_cpu *cpu){
    if(cpu->pcb->socket_memoria<0){return;}
    //recibo del carpincho
    int dest=recibir_codigo(cpu->pcb->socket_carpincho);
    int size=recibir_codigo(cpu->pcb->socket_carpincho);
    void *dato=recibir_buffer(&size,cpu->pcb->socket_carpincho);
    log_info(logger, ">> Se recibe un memwrite del carpincho %d\n", cpu->pcb->pid);
    //envio a memoria
    enviar_codigo(memwrite,cpu->pcb->socket_memoria);
    enviar_codigo(dest,cpu->pcb->socket_memoria);
    enviar_codigo_y_datos(size,dato,size,cpu->pcb->socket_memoria);
    cpu->resp=recibir_codigo(cpu->pcb->socket_memoria);
    //return;
}

void manejo_mateclose(t_cpu *cpu){
    if(cpu->pcb->socket_memoria>0){
        enviar_codigo(MATECLOSE,cpu->pcb->socket_memoria);
    }
    log_info(logger,">> Se recibe un mateclose del carpincho %i, finaliza",cpu->pcb->pid);
    sem_wait(sem_planificador);
    cpu->estado=LIBRE;
    ejecutar_planificador_largo(cpu->pcb);
    sem_post(sem_planificador);
    //return;
}

void manejo_seminit(t_cpu *cpu){
    char *nombre=recibir_datos(cpu->pcb->socket_carpincho);
    int valor=recibir_codigo(cpu->pcb->socket_carpincho);
    log_info(logger,">> Se recibe un seminit para el semaforo %s del caprincho %i con valor %i",nombre,cpu->pcb->pid,valor);
    sem_wait(sem_lista_semaforos);
    //aca deberia validar que no exita el semaforo con ese nombre y creo uno nuevo
    
    t_semaforo* nuevo_semaforo = busca_semaforo(nombre);
    if( nuevo_semaforo == NULL){
        nuevo_semaforo = malloc(sizeof(t_semaforo));
        nuevo_semaforo->lista_espera = list_create();
        nuevo_semaforo->lista_usando = list_create();
        nuevo_semaforo->mi_semaforo = malloc(sizeof(sem_t));
        sem_init(nuevo_semaforo->mi_semaforo,0,1);
        nuevo_semaforo->nombre = nombre;
        nuevo_semaforo->valorIncial = valor;
        nuevo_semaforo->valorActual = valor;
        list_add(lista_semaforos,nuevo_semaforo);
    }
    else{
        log_info(logger,"El semaforo %s ya existe",nombre);
        cpu->resp=MAL;
        free(nombre);
    } 
    sem_post(sem_lista_semaforos);
    cpu->resp=OK;//respuesta que se envia al carpincho
    //return;
}

void manejo_semwait(t_cpu *cpu){
    cpu->resp=OK;
    char *nombre=recibir_datos(cpu->pcb->socket_carpincho);
    log_info(logger,">> Se recibe un semwait para el semaforo %s del caprincho %i",nombre,cpu->pcb->pid);
    sem_wait(sem_lista_semaforos);
    /*
    *
    *si es un semaforo valido y tiene valor menor a 1
    *tengo que agregar el pcb a la lista de espera del semaforo restar 1 al contador
    *free(nombre);
    * y tengo que ejecutar interrupcion_llamada_bloqueante(cpu) para bloquear y replanificar
    * si es 1+ tengo que agregarlo a la lista_usando del semaforo
    * y hacer sem_post(cpu->mi_semaforo) para que continue la ejecucion
    */

    t_semaforo* semaforoBuscado = busca_semaforo(nombre);
    if (semaforoBuscado == NULL){
        log_info(logger,"El semaforo %s no existe",nombre);
        cpu->resp=MAL;
        sem_post(sem_lista_semaforos);
        sem_post(cpu->mi_semaforo);
        free(nombre);
        return;
    }
    sem_wait(semaforoBuscado->mi_semaforo);
    if (semaforoBuscado->valorActual > 0){
        semaforoBuscado->valorActual--;
        list_add(semaforoBuscado->lista_usando,cpu->pcb);
        // 4deadlock
        list_add(cpu->pcb->lista_semaforos_asignados,semaforoBuscado);
        
        sem_post(semaforoBuscado->mi_semaforo);
        sem_post(cpu->mi_semaforo);
        sem_post(sem_lista_semaforos);
        free(nombre);
        return;
    }

    else{
        semaforoBuscado->valorActual--;
        list_add(semaforoBuscado->lista_espera,cpu->pcb);
        sem_post(semaforoBuscado->mi_semaforo);
        sem_post(sem_lista_semaforos);
        cpu->pcb->semaforo_espero=semaforoBuscado;
        // 4deadlock
        sem_wait(sem_lista_semaforos_bloqueados);
        list_add(lista_bloqueados_por_semaforos,cpu->pcb);
        sem_post(sem_lista_semaforos_bloqueados);

        interrupcion_llamada_bloqueante(cpu);
        free(nombre);
        return;
    }
}

void manejo_sempost(t_cpu *cpu){
    char *nombre=recibir_datos(cpu->pcb->socket_carpincho);
    log_info(logger,">> Se recibe un sempost para el semaforo %s del caprincho %i",nombre,cpu->pcb->pid);
    sem_wait(sem_lista_semaforos);
    /*
    *si es un semaforo valido tengo que agregarlo a la lista_usando del semaforo
    *y sumar 1 al contador
    */
    t_semaforo* semaforoBuscado = busca_semaforo(nombre);
    if (semaforoBuscado == NULL){
        log_info(logger,"# El semaforo %s no existe",nombre);
        cpu->resp=MAL;
        sem_post(sem_lista_semaforos);
        free(nombre);
        return;
    }
    sem_wait(semaforoBuscado->mi_semaforo);
    semaforoBuscado->valorActual++;
    quitar_de_lista(semaforoBuscado->lista_usando,cpu->pcb);
    quitar_de_lista(cpu->pcb->lista_semaforos_asignados,semaforoBuscado);

    if(list_size(semaforoBuscado->lista_espera) > 0){   
        semaforoBuscado->valorActual--;
        t_pcb* pcb = list_remove(semaforoBuscado->lista_espera,0);
        list_add(pcb->lista_semaforos_asignados,semaforoBuscado);
        pcb->semaforo_espero=NULL;
        quitar_de_lista(lista_bloqueados_por_semaforos,pcb);
        list_add(semaforoBuscado->lista_usando,pcb);
        interrupcion_fin_bloqueo(pcb);
    }
    sem_post(semaforoBuscado->mi_semaforo);

    // 4deadlock
    sem_wait(sem_lista_semaforos_bloqueados);
    quitar_de_lista(lista_semaforos_bloqueados,semaforoBuscado);
    sem_post(sem_lista_semaforos_bloqueados);

    free(nombre);
    sem_post(sem_lista_semaforos);
    cpu->resp=OK;
    //return;
}

void manejo_semdestroy(t_cpu *cpu){
    cpu->resp=OK;
    char *nombre=recibir_datos(cpu->pcb->socket_carpincho);
    log_info(logger,">> Se recibe un semdestroy para el semaforo %s del caprincho %i",nombre,cpu->pcb->pid);
    sem_wait(sem_lista_semaforos);
    /*
    *validar que no este en uso, quitar de listas y hacer free a todos los punteros
    *y al semaforo
    */
    t_semaforo* semaforoBuscado = busca_semaforo(nombre);
    if (semaforoBuscado == NULL){
        log_info(logger,"# El semaforo %s no existe",nombre);
        cpu->resp=MAL;
        sem_post(sem_lista_semaforos);
        free(nombre);
        return;
    }

    if (semaforoBuscado->valorActual != semaforoBuscado->valorIncial){
        log_info(logger,"# El semaforo %s esta en uso",nombre);
        cpu->resp=MAL;
        sem_post(sem_lista_semaforos);
        free(nombre);
        return;
    }
    quitar_de_lista(lista_semaforos,semaforoBuscado);

    //TODO: Hacer en una funcion el destroy
    list_destroy(semaforoBuscado->lista_espera);
    list_destroy(semaforoBuscado->lista_usando);
    free(semaforoBuscado->mi_semaforo);
    free(semaforoBuscado);
    
    sem_post(sem_lista_semaforos);
    sem_post(cpu->mi_semaforo);
    free(nombre);
    //return;
}

void manejo_mateio(t_cpu *cpu){
    char *nombre=recibir_mensaje(cpu->pcb->socket_carpincho);
    t_io *io=busca_io(nombre);
    if (io!=NULL){
        free(nombre);
        log_info(logger,">> Se recibe del carpincho %i mateio en el dispositivo %s",cpu->pcb->pid,io->nombre);
        sem_wait(io->sem_lista);
        list_add(io->lista_espera,cpu->pcb);
        sem_post(io->sem_lista);
        sem_post(io->mi_semaforo);
        
        return interrupcion_llamada_bloqueante(cpu);
    }
    sem_post(cpu->mi_semaforo);
    log_warning(logger,"# El carpincho [%i] ejecuta el IO [%s] que no existe",cpu->pcb->pid,nombre);
    cpu->estado=MAL;
    free(nombre);
   // return;
}

void borra_semaforo(char *nombre){
    t_semaforo *sem;
    for(int i=0;i<list_size(lista_semaforos);i++){
        sem=list_get(lista_semaforos,i);
        if (strcmp(sem->nombre,nombre)==0){
            sem=list_remove(lista_semaforos,i);
            sem_destroy(sem->mi_semaforo);
            list_destroy(sem->lista_espera);
            free(sem->nombre);
            free(sem);
            return;
        }
    }
}

t_semaforo *busca_semaforo(char *nombre){
    t_semaforo *sem =NULL;
    for(int i=0;i<list_size(lista_semaforos);i++){
        sem=list_get(lista_semaforos,i);
        if (strcmp(sem->nombre,nombre)==0){return sem;}
    }
    return NULL;
}

t_semaforo *busca_semaforo_en(t_list* lista,t_semaforo* semaforo){
    t_semaforo *sem =NULL;
    for(int i=0;i<list_size(lista);i++){
        sem=list_get(lista,i);
        if (sem == semaforo){
            return sem;
        }
    }
    return NULL;
}

t_io *busca_io(char *nombre){
    t_io *io =NULL;
    for(int i=0;i<list_size(lista_io);i++){
        io=list_get(lista_io,i);
        if (strstr(io->nombre,nombre)!=NULL){
            if(strcmp(strstr(io->nombre,nombre),nombre)==0){
                return io;
            }
        }
    }
    return NULL;
}

void *manejo_io(t_io *disp){//funcion que maneja el hilo de dispositivos IO
    log_info(logger,"---> inicio el dipositivo [%s] con delay [%i]",disp->nombre,disp->delay);
    t_pcb *pcb;
    while(1){
        sem_wait(disp->mi_semaforo);
        sem_wait(disp->sem_lista);
        pcb=list_remove(disp->lista_espera,0);
        sem_post(disp->sem_lista);
        log_info(logger," # El carpincho [%i] esta usando el dispositivo [%s]",pcb->pid,disp->nombre);
        usleep(disp->delay*1000);
        log_info(logger,"# El carpincho [%i] libera el dispositivo [%s]",pcb->pid,disp->nombre);
        interrupcion_fin_bloqueo(pcb);
    }
    return NULL;
}


 void interrupcion_fin_bloqueo(t_pcb *pcb){
    //desbloqueo el pcb y ejecuto planificadores
    sem_wait(sem_planificador);
    if (pcb->estado==SUSPENDEDBLOCKED){
        ejecutar_planificador_mediano(pcb);
        sem_post(sem_planificador);
        return;
    }
    ejecutar_planificador_corto(pcb);
    sem_post(sem_planificador);
 }

void interrupcion_llamada_bloqueante(t_cpu *cpu){
     //bloqueo el pcb y ejecuto planificadores
    sem_wait(sem_planificador);
    cpu->estado=LIBRE;
    ejecutar_planificador_corto(cpu->pcb);
    sem_post(sem_planificador);
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
