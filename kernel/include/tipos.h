#ifndef TIPOS_H
#define TIPOS_H

typedef struct {
    char *IP_MEMORIA;
    char *PUERTO_MEMORIA;
    char *PUERTO_ESCUCHA;
    char *ALGORITMO_PLANIFICACION;
    int ESTIMACION_INICIAL;
    float ALFA;
    char *DISPOSITIVOS_IO;
    char *DURACIONES_IO;
    int GRADO_MULTIPROGRAMACION;
    int GRADO_MULTIPROCESAMIENTO;
    int TIEMPO_DEADLOCK;
    char * IP_LOCAL;
} t_kernel_config;

typedef enum{
    SJF,
    HRRN,
}cod_algoritmo;
cod_algoritmo algoritmo;


typedef enum{
    NEW,
    READY,
    BLOCKED,
    SUSPENDEDBLOCKED,
    SUSPENDEDREADY,
    EXECUTING,
    EXIT,
}cod_estados;

typedef struct{
    char *nombre;
    int valorActual;
    int valorIncial;
    sem_t *mi_semaforo;
    t_list *lista_usando;
    t_list *lista_espera;
}t_semaforo;//un semaforo virtual

typedef struct{
    u_int32_t pid;
    clock_t estimacionAnterior;
    clock_t duracionRealAnterior;
    clock_t instanteEstado;//valor de clock del momento que ingresa a un estado
    cod_estados estado;
    t_list* lista_semaforos_asignados;
    t_semaforo *semaforo_espero;
    int socket_carpincho;
    int socket_memoria;
}t_pcb;//el pcb para el manejo de los procesos

u_int32_t contPid; 


sem_t *sem_lista_semaforos;
sem_t *sem_lista_semaforos_bloqueados;
t_list *lista_semaforos; ///lista con los semaforos existentes
t_list *lista_semaforos_bloqueados;
t_list* lista_deadlock;
t_list *lista_bloqueados_por_semaforos;
typedef struct{
    char *nombre;
    int delay;
    t_pcb *pcb;
    pthread_t *hilo;
    sem_t *mi_semaforo;
    sem_t *sem_lista;
    t_list *lista_espera;
}t_io;//dispositivo io
//sem_t *sem_io;//semaforos para controlar los ip
int cant_dispositivos;//cantidad de dispositivos io
t_list *lista_io; //lista con los dispositivos

t_kernel_config *kernel_config;
t_log *logger;


//listas de estados
t_list *lista_new;
t_list *lista_ready;
t_list *lista_blocked;
t_list *lista_executing;
t_list *lista_suspended_blocked;
t_list *lista_suspended_ready;
t_list *lista_exit;


typedef struct{
    sem_t *mi_semaforo;//semaforo para controlar ejecucion
    pthread_t *hilo;//hilo donde ejecuta el cpu
    t_pcb *pcb;//pcb del proceso en ejecucion
    t_libre estado;//si el cpu esta disponible para un nuevo proceso
    int numero;
    int resp;
}t_cpu;

t_cpu *cpus;

sem_t *sem_planificador;//mutex para ejecutar planificadores

#endif