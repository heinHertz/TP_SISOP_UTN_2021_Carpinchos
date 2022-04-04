#ifndef MANEJOMEMORIA_H
#define MANEJOMEMORIA_H
enum mate_errors {
    MATE_FREE_FAULT = -5,
    MATE_READ_FAULT = -6,
    MATE_WRITE_FAULT = -7
};

int metodo_asignacion;
int metodo_reemplazo_tlb;
int metodo_reemplazo_mmu;

uint32_t obtener_frame(uint32_t pagina,t_pcb *pcb);

void iniciar_ram(void);

uint32_t tamHeap;
uint32_t buscar_hueco (uint32_t size, t_pcb *PCB);
int leer(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB);
int validar_leer(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB);
int escribir(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB);
int validar_escribir(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB);
int liberar(uint32_t direccion,t_pcb *PCB);

uint32_t direccionFisica(uint32_t dirLogica,t_pcb *PCB);
int cabe(uint32_t size,uint32_t dirheap,t_alloc heap);
uint32_t busca_en_tabla(uint32_t nroPagina,t_pcb *PCB);
u_long inc_inst (void);
uint32_t traer (uint32_t pagina,t_pcb *PCB);
int busca_en_TLB(uint32_t paginaBuscada,uint32_t *frame,t_pcb *PCB);
uint32_t nuevo_alloc(uint32_t size, t_alloc heap,uint32_t dirheap,t_pcb *PCB);
int primer_alloc(t_pcb *PCB);

int asignarPaginas(u_int32_t cant,t_pcb *PCB);
int asignarPaginas_dinamico(u_int32_t cant,t_pcb *PCB);
int asignarPaginas_fijo(u_int32_t cant,t_pcb *PCB);


void unir_heap(t_alloc heap,uint32_t posicion, t_pcb *PCB);

void agrego_a_TLB(uint32_t pagina,uint32_t frame,t_pcb *PCB);
uint32_t buscar_victima_tlb(void);

void suspender_proceso(t_pcb *pcb);
void enviar_victima(t_pcb *PCB,uint32_t pagVictima);
uint32_t buscar_victima_tabla(int nroPagina,t_pcb *PCB);
uint32_t buscar_victima_tabla_fijo(int nroPagina,t_pcb *PCB);
uint32_t buscar_victima_tabla_fijo_algoritmo_clockM(int nroPagina,t_pcb *PCB);
uint32_t buscar_victima_tabla_fijo_algoritmo_LRU(int nroPagina,t_pcb *PCB);
uint32_t buscar_victima_tabla_dinamico(int nroPagina,t_pcb *PCB);
uint32_t buscar_victima_tabla_dinamico_algoritmo_clockM(int nroPagina,t_pcb *PCB);
uint32_t buscar_victima_tabla_dinamico_algoritmo_LRU(int nroPagina,t_pcb *PCB);

void borrar_De_TLB(uint32_t pid,uint32_t pagina);

void nuevo_miss(u_int32_t pidBuscado);
void nuevo_hit(u_int32_t pidBuscado);
void * buscar_contador(u_int32_t pid);

#endif