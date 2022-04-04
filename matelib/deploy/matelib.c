#include <matelib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <commons/config.h>
#include "server.h"
#include "codigos_operacion.h"
#include "server.c"

//------------------General Functions---------------------/
typedef enum{
  DISPONIBLE,
  NODISPONIBLE,
  ERROR,
}conectado_a;

t_config *leer_config(char *config){
  t_config *temp=config_create(config);
if (temp==NULL){
    temp=config_create("carpincho.config");
}
return temp;

};


typedef struct mate_inner_structure{
  int socket_servidor;
  conectado_a kernel;
} mate_inner_structure;

int mate_init(mate_instance *lib_ref, char *config){
  t_config *conf=NULL;
  conf= leer_config(config);
   if (conf==NULL){
    perror("Falta yerba!!!\nno recibi un config valido ni pude leer el archivo ./carpincho.config ejemplo valido:\n");
    printf("IP_KERNEL=127.0.0.1\n");
    printf("PUERTO_KERNEL=6001\n");
    printf("IP_MEMORIA=127.0.0.1\n");
    printf("PUERTO_MEMORIA=5001\n");
    return -1;
  }
  mate_inner_structure *estructura=malloc(sizeof(mate_inner_structure));
  estructura->kernel=DISPONIBLE;
  //int socket= abrir_socket_servidor(config_get_string_value(conf,"IP_KERNEL"),config_get_string_value(conf,"PUERTO_KERNEL"));
  int socket= conectar_a(config_get_string_value(conf,"IP_KERNEL"),config_get_string_value(conf,"PUERTO_KERNEL"));
  if (socket<0){
    perror("no pude conectar con kernel, intento con memoria\n");
    estructura->kernel=NODISPONIBLE;
    socket= conectar_a(config_get_string_value(conf,"IP_MEMORIA"),config_get_string_value(conf,"PUERTO_MEMORIA"));
  }
  if (socket <0){
        perror("no pude conectar a memoria tampoco, no hay mate para vos\n");
      free(estructura);
      lib_ref->group_info =NULL;
      return -1;
  }
  
  estructura->socket_servidor=socket;
  lib_ref->group_info = estructura;
  //espero habilitacion para continuar
  int respuesta=recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  if(respuesta==0){
  return 0;
  }
  perror("algo salio mal del otro lado :v\n");
  return -1;
}

int mate_close(mate_instance *lib_ref)
{
  if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
  }
  enviar_codigo(MATECLOSE,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  close (((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  free(lib_ref->group_info);
  //printf("se acabo la yerba, fin del mate\n");
  return 0;
}

//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value) {
  if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
  }
  if (((mate_inner_structure *)lib_ref->group_info)->kernel==NODISPONIBLE){
    perror("no hay bizcochitos -Kernel-, no esta disponible la planificacion");
    return -1;
  }
  //envio el nombre y luego el valor del semaforo (para no complicarla)
 
  enviar_codigo_y_datos(SEMINIT,sem,strlen(sem)+1,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  enviar_codigo(value,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem) {
  if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
  }
  if (((mate_inner_structure *)lib_ref->group_info)->kernel==NODISPONIBLE){
    perror("no hay bizcochitos -Kernel-, no esta disponible la planificacion");
    return -1;
  }
  enviar_codigo_y_datos(SEMWAIT,sem,strlen(sem)+1,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  int cod=recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  if (cod==-9){
    printf("me finalizaron\n");
    close (((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  }
  return cod;
 
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem) {
  if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
  }
  if (((mate_inner_structure *)lib_ref->group_info)->kernel==NODISPONIBLE){
    perror("no hay bizcochitos -Kernel-, no esta disponible la planificacion"); 
    return -1;
  }
  enviar_codigo_y_datos(SEMPOST,sem,strlen(sem)+1,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);

}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem) {
  if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
  }
  if (((mate_inner_structure *)lib_ref->group_info)->kernel==NODISPONIBLE){
    perror("no hay bizcochitos -Kernel-, no esta disponible la planificacion"); 
    return -1;
  }
  enviar_codigo_y_datos(SEMDESTROY,sem,strlen(sem)+1,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
}

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg){
  if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
  }
  if (((mate_inner_structure *)lib_ref->group_info)->kernel==NODISPONIBLE){
    perror("no hay bizcochitos -Kernel-, no esta disponible la planificacion"); 
    return -1;
  }
  enviar_codigo(MATEIO,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  enviar_mensaje(io,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
  
  return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);

}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size){
    if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
    }
    if(size<1){return MATE_WRITE_FAULT;}
    enviar_codigo(memalloc,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    enviar_codigo(size,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr){
    if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
    }
    if (addr<1){return MATE_FREE_FAULT;}
    enviar_codigo(memfree,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    enviar_codigo(addr,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);

}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size){
    if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
    }
    if (origin<1 || size<1){return MATE_READ_FAULT;}
    enviar_codigo(memread,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    enviar_codigo(origin,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    enviar_codigo(size,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    int resp=recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
      if (resp>=0){
        void *data=recibir_datos(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
        memcpy(dest,data,size);
        free(data);
        recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
      }
    return resp;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size){
    if(lib_ref->group_info == NULL){
    perror("la instancia del mate no es valida, inicia el mate primero");
    return -1;
    }
     if (dest<1 || size<1){return MATE_WRITE_FAULT;}
    enviar_codigo(memwrite,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    enviar_codigo(dest,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    enviar_codigo_y_datos(size,origin,size,((mate_inner_structure *)lib_ref->group_info)->socket_servidor);
    return recibir_codigo(((mate_inner_structure *)lib_ref->group_info)->socket_servidor);

  
}