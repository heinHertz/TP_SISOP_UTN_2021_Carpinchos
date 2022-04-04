#ifndef TIPOS_H
#define TIPOS_H
#include <stdint.h>

typedef struct{
    void *pcb;
	uint32_t frame;
	uint32_t valido;
	uint32_t Modificado;
	u_long accedido;
    uint32_t indice;
	//uint32_t clock;
} t_tabla_paginas;

typedef struct heapmetadata{
    uint32_t prevAlloc;
    uint32_t nextAlloc;
    uint8_t isFree;
} __attribute__((packed)) t_alloc;
typedef struct {
    uint32_t pid;
    t_tabla_paginas *tabla_paginas;
    uint32_t alocado;
    uint32_t paginas;
    int socket;
    uint32_t clock;
    uint32_t inicioVuelta;
    int vuelta;
    pthread_t hilo;
    t_tabla_paginas **frames;//para fijo
} t_pcb;



typedef struct{
	uint32_t pid;
	uint32_t pagina;
    uint32_t frame;
	u_long cont;
} t_tlb;

typedef struct{
	uint32_t pid;
	u_long count_tlb_hit;
	u_long count_tlb_miss;
}t_contadores;



typedef struct {
    char* IP;
    char* PUERTO;
    int TAMANIO;
    int TAMANIO_PAGINA;
    char* TIPO_ASIGNACION;
    char* ALGORITMO_REEMPLAZO_MMU;
    int CANTIDAD_ENTRADAS_TLB;
    char* ALGORITMO_REEMPLAZO_TLB;
    int RETARDO_ACIERTO_TLB;
    int RETARDO_FALLO_TLB;
    int MARCOS_POR_CARPINCHO;
    char *PATH_DUMP_TLB;
} t_memoria_config;

typedef enum{
    FIFO,
    FIJA,
    DINAMICA,
    LRU,
    CLOCKM,
}t_init;
uint32_t pid;
#endif //TIPOS_H