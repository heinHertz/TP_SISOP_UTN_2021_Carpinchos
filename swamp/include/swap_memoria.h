#ifndef SWAP_MEMORIA_H
#define SWAP_MEMORIA_H

#include <stdio.h>

#include "stdint.h"
#include <stdio.h>
#include <fcntl.h>

#include <stdlib.h>
#include <unistd.h>
#include "swamp_config.h"

#include <commons/string.h>

typedef struct
{
	int32_t* free;
	int32_t posicion;
	uint32_t * pagina;
} t_pagina;

t_list * paginas_swap; 

uint32_t global_total_paginas;

void guardar_pagina( int proceso );

#endif