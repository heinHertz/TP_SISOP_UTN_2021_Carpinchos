#include "kernel.h"

int run_tests(){
    CU_initialize_registry();
    CU_pSuite tests = CU_add_suite("KERNEL Suite",NULL,NULL);
    CU_add_test(tests,"Probar Suma", suma_kernel);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
void suma_kernel(){
    CU_ASSERT_EQUAL(2+2, 4);
}

void pruebaAlgoritmos(){

    t_pcb* pcb1 = malloc (sizeof(t_pcb));
    pcb1->pid = 1;
    pcb1->duracionRealAnterior = 0;
    pcb1->estimacionAnterior = 2000;
    list_add(lista_ready,pcb1);
    ejecutar_planificador_corto(pcb1);
    sleep(2);

    t_pcb* pcb2 = malloc (sizeof(t_pcb));
    pcb2->pid = 2;
    pcb2->duracionRealAnterior = 0;
    pcb2->estimacionAnterior = 2000;
    list_add(lista_ready,pcb2);
    ejecutar_planificador_corto(pcb2);
    sleep(3);

    t_pcb* pcb3 = malloc (sizeof(t_pcb));
    pcb3->pid = 3;
    pcb3->duracionRealAnterior = 0;
    pcb3->estimacionAnterior = 2000;
    list_add(lista_ready,pcb3);
    ejecutar_planificador_corto(pcb3);
    sleep(4);

    planificar_SJF_test();
    planificar_HRRN_test();

}

void planificar_SJF_test(){
    t_pcb* _menorSJF ( void* _pcb1,void* _pcb2)    
    {
        t_pcb* pcb1 = (t_pcb*)_pcb1;
        t_pcb* pcb2 = (t_pcb*)_pcb2;
        return menorSJF(pcb1,pcb2);
    };
    t_pcb* pcb_menorRafaga = list_get_minimum(lista_ready,_menorSJF);
    quitar_de_lista(lista_ready,pcb_menorRafaga);
    pcb_menorRafaga->estimacionAnterior = calculoEstimado(pcb_menorRafaga->duracionRealAnterior,pcb_menorRafaga->estimacionAnterior);
    printf("SFJ: PID: %d",pcb_menorRafaga->pid);
}

void planificar_HRRN_test(){
    t_pcb* _mayorRR ( void* _pcb1,void* _pcb2)    
    {
        t_pcb* pcb1 = (t_pcb*)_pcb1;
        t_pcb* pcb2 = (t_pcb*)_pcb2;
        return mayorRR(pcb1,pcb2);
    };
    t_pcb* pcb_mayorRR = list_get_maximum(lista_ready,_mayorRR);
    quitar_de_lista(lista_ready,pcb_mayorRR);
    pcb_mayorRR->estimacionAnterior = calculoEstimado(pcb_mayorRR->duracionRealAnterior,pcb_mayorRR->estimacionAnterior);
    printf("SFJ: PID: %d",pcb_mayorRR->pid);
}

void pruebaDeadlock(){

    t_semaforo* semaforo0 = malloc(sizeof(t_semaforo));
    semaforo0->lista_espera = list_create();
    semaforo0->lista_usando = list_create();
    semaforo0->mi_semaforo = malloc(sizeof(sem_t));
    sem_init(semaforo0->mi_semaforo,0,1);
    semaforo0->nombre = "semaforo0";
    semaforo0->valorIncial = 1;
    semaforo0->valorActual = 0;
    
        
    t_semaforo* semaforo1 = malloc(sizeof(t_semaforo));
    semaforo1->lista_espera = list_create();
    semaforo1->lista_usando = list_create();
    semaforo1->mi_semaforo = malloc(sizeof(sem_t));
    sem_init(semaforo1->mi_semaforo,0,1);
    semaforo1->nombre = "semaforo1";
    semaforo1->valorIncial = 1;
    semaforo1->valorActual = 0;
    

    t_semaforo* semaforo2 = malloc(sizeof(t_semaforo));
    semaforo2->lista_espera = list_create();
    semaforo2->lista_usando = list_create();
    semaforo2->mi_semaforo = malloc(sizeof(sem_t));
    sem_init(semaforo2->mi_semaforo,0,1);
    semaforo2->nombre = "semaforo2";
    semaforo2->valorIncial = 1;
    semaforo2->valorActual = 0;
    

    t_pcb* pcb1 = malloc (sizeof(t_pcb));
    pcb1->pid = 1;
    pcb1->lista_semaforos_asignados = list_create();
    pcb1->estado = BLOCKED;
    list_add(pcb1->lista_semaforos_asignados,semaforo2);


    t_pcb* pcb2 = malloc (sizeof(t_pcb));
    pcb2->pid = 2;
    pcb2->lista_semaforos_asignados = list_create();
    pcb2->estado = BLOCKED;
    list_add(pcb2->lista_semaforos_asignados,semaforo0);

    t_pcb* pcb3 = malloc (sizeof(t_pcb));
    pcb3->pid = 3;
    pcb3->lista_semaforos_asignados = list_create();
    pcb3->estado = BLOCKED;
    list_add(pcb3->lista_semaforos_asignados,semaforo1);

    list_add(semaforo0->lista_espera,pcb1);
    list_add(semaforo0->lista_usando,pcb2); 

    list_add(semaforo1->lista_espera,pcb2);
    list_add(semaforo1->lista_usando,pcb3);

    list_add(semaforo2->lista_espera,pcb3);
    list_add(semaforo2->lista_usando,pcb1);

    list_add(lista_semaforos_bloqueados,semaforo0);
    list_add(lista_semaforos_bloqueados,semaforo1);
    list_add(lista_semaforos_bloqueados,semaforo2);

    _iniciar_detector_deadlock();
    /*
    lista_semaforos_bloqueados[semaforo0,semaforo1,semaforo2]
    semaforo 0
    |->  pcb1 -> lista de semaforos tomados [semaforo2]      

    semaforo 1
    |->  pcb2 -> lista de semaforos tomados [semaforo0]

    semaforo 2
    |->  pcb3 -> lista de semaforos tomados [semaforo1]
    */
    
}