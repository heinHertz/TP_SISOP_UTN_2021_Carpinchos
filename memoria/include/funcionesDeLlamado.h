#ifndef FUNCIONESDELLAMADO_H
#define FUNCIONESDELLAMADO_H

void iniciar_carpincho(int socket_cliente);
void *atiende_carpincho(t_pcb *pcb);

//cuando un nuevo carpincho se conecta, antes debe crearse un pcb y pasarlo por parametro
//devuelve 0 si pudo asignarse o -1 si no puede crear las estructuras de memoria (en caso de asignacion fija)
int nuevo_proceso_ram(t_pcb *PCB);

//se invoca cuando se llama a mate_memalloc() recibe tamaño enviado y PCB del carpincho (previamente iniciado con nuevo_proceso_ram)
// devuelve 0 si no puede alocar memoria, y > 0 que corresponde a la direccion asignada
uint32_t alocar_memoria(uint32_t tamanio, t_pcb *PCB);

//se invocaria con mate_memread, direccion desde donde lee, y cuantos bytes lee, y el PCB del carpincho
//devuelve en el puntero dato el valor leido (se debe alocar previamente y realizar free() luego de usarlo)
//la funcion devuelve 0 si pudo leer y -1 si no pudo
int leer_memoria(void *dato,uint32_t inicio,uint32_t tamanio, t_pcb *PCB);

//idem a leer, pero recibe un puntero con el dato a escribir, tambien deberia hacer free al dato
int escribir_memoria(void *dato,uint32_t inicio,uint32_t tamanio, t_pcb *PCB);

//recibe la direccion a liberar (puntero) y el pcb falta definir valores de retorno
int liberar_memoria(uint32_t direccion,t_pcb *PCB);


//enviar a swamp el pedido y devuelve 0 si pudo reservar, -1 si no pudo
//el puntero listaFrames deberia apuntar a la lista devuelta por swamp
int reservar_frames_swap(uint32_t cant,t_pcb *PCB);

//recibe numero de pagina a enviar a swamp, y devuelve en el puntero el contenido retornado
void *traer_pagina(u_int32_t nroPagina,t_pcb *PCB);
uint32_t enviar_pagina(void *contenido,uint32_t nroPagina, t_pcb *PCB);

void *atiende_carpincho(t_pcb *pcb);

#endif