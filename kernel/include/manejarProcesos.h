#ifndef MANEJARPROCESOS_H
#define MANEJARPROCESOS_H
#include "kernel.h"
void *manejo_cpu(t_cpu *cpu);


void *manejo_io(t_io *cpu);
t_io *busca_io(char *nombre);
void interrupcion_fin_bloqueo(t_pcb *pcb);
void interrupcion_llamada_bloqueante(t_cpu *cpu);
void manejo_memalloc(t_cpu *cpu);
void manejo_memfree(t_cpu *cpu);
void manejo_memread(t_cpu *cpu);
void manejo_memwrite(t_cpu *cpu);
void manejo_mateclose(t_cpu *cpu);
void manejo_seminit(t_cpu *cpu);
void manejo_semwait(t_cpu *cpu);
void manejo_sempost(t_cpu *cpu);
void manejo_semdestroy(t_cpu *cpu);
void manejo_mateio(t_cpu *cpu);            

t_semaforo *busca_semaforo(char *nombre);
t_semaforo *busca_semaforo_en(t_list* lista,t_semaforo* semaforo);
void quitar_de_lista(t_list *lista, void *ptr);

#endif