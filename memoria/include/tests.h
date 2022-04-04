#ifndef TESTS_H
#define TESTS_H
#include <commons/bitarray.h>
//#include "memoria.h" //FIXME: Comente la linea porque daba error, ya que memoria.h ya incluye a test.h y se queda recursivo 

int tamswamp;
char *swamp;
t_bitarray* bitMapSwamp;
char *punteroBitMapS;
void allocs(t_pcb *PCB);
int *reservar_frames_test(uint32_t cant,t_pcb *PCB);
void *traer_pagina_test(u_int32_t nroPagina,t_pcb *PCB);
uint32_t enviar_pagina_test(void *contenidoSwap,t_pcb *PCB);

void* tests_mensajes_swamp();
void* enviar_mensajes_prueba_swamp();

#endif