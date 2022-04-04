#ifndef CODIGOS_OPERACION_H
#define CODIGOS_OPERACION_H

typedef enum{
    LIBRE,
    OCUPADO
}t_libre;
typedef enum{
    MAL=-1,
    OK=0
}resp_code;


//Codigos conexion Memoria con SWAMP
typedef enum {
    OP_MENSAJE_SIMPLE  = 0,
    OP_MENSAJES_SIMPLES = 1,
    OP_INICIAR_PROCESO = 3,
    OP_TIPO_ASIGNACION_MEMORIA = 4,
    OP_PEDIR_PAGINA   = 5,
    OP_GUARDAR_PAGINA = 6,
    OP_ESCRIBIR_PAGINA = 7,
    OP_RESERVAR_PAGINAS = 8,
    OP_FINALIZAR_PROCESO = 9,
    OP_FINALIZAR_CONEXION_SWAMP = 11
} ops_code;

//Codigos conexion MateLib
typedef enum tipo_mensaje{
    memalloc = 0,
    memfree = 1,
    memread = 2,
    memwrite = 3,
    memsuspend=4,
    conexion_swamp = 9,
    MATEINIT=10,
    MATECLOSE=11,
    SEMINIT=12,
    SEMWAIT=13,
    SEMPOST=14,
    SEMDESTROY=15,
    MATEIO=16
} tipo_mensaje_t;

typedef enum
{
	TIPO_ASIGNACION_FIJA = 1,
	TIPO_ASIGNACION_DINAMICA = 2,
}t_asignacion;


#endif

