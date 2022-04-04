#ifndef PLANIFICADORES_H
#define PLANIFICADORES_H
#include "kernel.h"
void ejecutar_planificador_largo(t_pcb *pcb);
void ejecutar_planificador_mediano(t_pcb *pcb);
void ejecutar_planificador_corto(t_pcb *pcb);

void iniciar_detector_deadlock(void);
void* _iniciar_detector_deadlock();
bool HayDeadLock (t_semaforo* semaforo);
void buscarDeadLock (t_list* lista_semaforos_asignados);
void aplicarPoliticaDeRecuperacion();
t_pcb* buscar_carpincho_mayor_ID (t_list* listaCarpinchos);


int debo_dessuspender(void);
int puedo_uno_mas(void);
int debo_suspender(void);
int tengo_procesos(void);

/**************************************************************/
void planificar_algoritmo(void);
void planificar_sjf(void);
t_pcb* _menorSJF (void* pcb1,void* pcb2);
//double calculoEstimado (clock_t duracionRealAnterior,clock_t estimacionAnterior);
double calculoEstimado (time_t duracionRealAnterior,time_t estimacionAnterior);
void planificar_hrrn(void);
t_pcb* mayorRR (t_pcb* pcb1,t_pcb* pcb2);
//clock_t calculoRR (clock_t tiempoEsperaEnReady,clock_t duracionRealAnterior,clock_t estimacionAnterior);
time_t calculoRR (time_t tiempoEsperaEnReady,time_t duracionRealAnterior,time_t estimacionAnterior);

void ejecutar(t_pcb *pcb);
int asigno_cpu(void);
int esperamissemaforos(t_pcb *pcb,t_pcb *pcb2);

#endif