#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <matelib.h>


void menu(void){
  printf("0: mateinit\n1: seminit\n2: semwait\n3: sempost\n4: semdestroy\n");
  printf("5: memalloc\n6: memwrite\n7: memread\n8: memfree\n9: mateio\n10: mateclose\n");
  }
void menusems(void){
  printf("0: sem1\n1: sem2\n2: sem3\n3: sem4\n5: menu\n");
  }
void menuio(void){
  printf("0: hierbitas\n1: laguna\n2: menu\n");
}
void menumem(void){
  printf("0: ptr1\n1: ptr2\n2: ptr3\n3: ptr4\n5: menu\n");
  }
mate_instance *idMate;
char *sem[5];
char *nptr[5];
int vptr[4];
//mate_sem_name sem;
int cod;
int value;
char *dato;
int main(int argc, char *argv[]){
  dato =malloc(1000);
sem[0]="sem1\0";
sem[1]="sem2\0";
sem[2]="sem3\0";
sem[3]="sem4\0";

nptr[0]="ptr1\0";
nptr[1]="ptr2\0";
nptr[2]="ptr3\0";
nptr[3]="ptr4\0";
for (int i=1;i<4;i++){vptr[i]=0;}

    while (1){
    menu();
    printf("ingresar opcion\n");
    scanf("%i",&cod);
    switch (cod){
        case 0:
          mateinit();
        break;
        case 1:
          sem_init();
          break;
        case 2:
          sem_wait();
          break;
        case 3:
          sem_post();
          break;
        case 4:
          sem_close();
          break;
        case 5:
          memalloc();
          break;
        case 6:
          memwrite();
          break;
        case 7:
          memread();
          break;
        case 8:
          memfree();
          break;
        case 9:
          mateio();
        break;  
        case 10:
          mateclose();
          break;
        default:
          printf("no es valido\n");
        break;  
    }


    }
}
void mateio(void){
  menuio();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  char *nombre;
  switch (cod){
  case 0:
    nombre="hierbitas";
    break;
  case 1:
    nombre="laguna";
    break;  
  default:
    return;
  }
  printf("uso el dispositivo %s\n");
  mate_call_io(idMate,nombre,"hola");

}

void memalloc(void){
  menumem();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("ingresar tamaño para alocar del puntero %s\n",nptr[cod]);
  scanf("%i",&value);
  vptr[cod]=mate_memalloc(idMate,value);
  printf("el puntero %s apunta a %i\n",nptr[cod],vptr[cod]);
  return;
}

void memwrite(void){
  menumem();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("ingresar dato a escribir para el puntero %s\n",nptr[cod]);
  scanf("%s",dato);
  int size=strlen(dato)+1;
  int res=mate_memwrite(idMate,dato,vptr[cod],size);
  if (res<0){printf("algo fallo");return res;}
  printf("escribi %s en el puntero %s, posicion %i\n",dato,nptr[cod],vptr[cod]);
  free(dato);
  dato = malloc(1000);
  return res;
}

void memread(void){
  menumem();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("ingresar tamaño a leer del puntero %s\n",nptr[cod]);
  scanf("%i",&value);
  int res=mate_memread(idMate,vptr[cod],dato,value);
  if (res<0){printf("algo fallo");return res;}
  printf("lei %s del puntero %s, posicion %i\n",dato,nptr[cod],vptr[cod]);
  free(dato);
  dato = malloc(1000);
  return res;
}

void memfree(void){
  menumem();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("voy a liberar el puntero %s",nptr[cod]);
  
  int res=mate_memfree(idMate,vptr[cod]);
  if (res<0){printf("algo fallo");return res;}
  printf("el puntero %s fue liberado",nptr[cod]);
  vptr[cod]=res;
  return;
}

void mateinit(void){
  idMate=malloc(sizeof(mate_instance));
  mate_init(idMate,"carpincho.config");
  printf("inicie el mate\n");
  return;
}

void sem_init(void){
  menusems();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("ingresar valor para el semaforo %s\n",sem[cod]);
  scanf("%i",&value);
  mate_sem_init(idMate,sem[cod],value);
  printf("inicie el semaforo\n");
  return;
}
void sem_wait(void){
  menusems();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("espero libere el semaforo %s",sem[cod]);
  mate_sem_wait(idMate,sem[cod]);
  printf("el semaforo se libero");
  return;
}
void sem_post(void){
  menusems();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("hago post al semaforo %s",sem[cod]);
  mate_sem_post(idMate,sem[cod]);
  printf("hice post");
  return;
}
void sem_close(void){
  menusems();
  printf("ingresar opcion\n");
  scanf("%i",&cod);
  if (cod==5){return;}
  printf("hago post al semaforo %s",sem[cod]);
  mate_sem_destroy(idMate,sem[cod]);
  printf("hice post");
  return;
}
void mateclose(void){
  if (cod==5){return;}
  
  mate_close (idMate);
  printf("hice fin del mate\n");
  return;
}