#include "memoria.h"

uint32_t metodo_memalloc(int pid, int size_solicitado){
    t_pcb* proceso;

    if(getProceso(pid) != NULL)
    {
        proceso = getProceso(pid);
    }
    else
    {
        proceso = malloc(sizeof(t_pcb));
        proceso->pid = pid;
        if (nuevo_proceso_ram(proceso) != -1) 
        {
            list_add(tablaDeProcesos,proceso);
        }
        else
        {
            log_info(logger,"Error al crear el proceso: %d \n",pid);
        }
        
    }

    uint32_t direccionLogica = buscar_hueco(size_solicitado,proceso);

    return direccionLogica;
}

bool existe(void * elemento, int pid)
{  
    t_pcb* proceso = (t_pcb*)elemento;
    return proceso->pid == pid;
}

t_pcb* getProceso (int pid){
   bool _existe ( void * elemento)    {
        return existe(elemento,pid);
    }

t_pcb* proceso = list_find(tablaDeProcesos,_existe);
return proceso;
}