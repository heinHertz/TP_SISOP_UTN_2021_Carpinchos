#include "kernel.h"

#include "planificadores.h"



void ejecutar_planificador_largo(t_pcb *pcb){

    if (pcb!=NULL){             //recibi un parametro

        if(pcb->estado==NEW){   //planifico uno nuevo

            pcb->pid=contPid++;

            pcb->estado=NEW;

            pcb->estimacionAnterior=kernel_config->ESTIMACION_INICIAL;

            pcb->duracionRealAnterior=0;    //valor rafaga anterior incial

            pcb->socket_memoria=conectar_a(kernel_config->IP_MEMORIA,kernel_config->PUERTO_MEMORIA);

            //pcb->instanteEstado=clock() / (CLOCKS_PER_SEC/1000);//instante que ingresa a new 

            pcb->instanteEstado=time(NULL) ;        //instante que ingresa a new 

            log_info(logger,"Carpincho [%i] se crea en estado NEW - estimacionAnterior:%d",pcb->pid, pcb->estimacionAnterior);

            if(pcb->socket_memoria>0){

                recibir_codigo(pcb->socket_memoria);

            }

            else{log_info(logger,"# La memoria no esta disponible para el carpincho %i",pcb->pid);}

            list_add(lista_new,pcb);   

            pcb->semaforo_espero=NULL;  

            pcb->lista_semaforos_asignados=list_create();  

        }

        else{   //es un fin

            pcb->estado=EXIT;

            close(pcb->socket_carpincho);

            close(pcb->socket_memoria);

            //pcb->duracionRealAnterior= (clock() / (CLOCKS_PER_SEC/1000) ) - pcb->instanteEstado;

            pcb->duracionRealAnterior= (time(NULL) ) - pcb->instanteEstado;

            log_info(logger,"Carpincho [%d] pasa a estado EXIT ",pcb->pid);

            quitar_de_lista(lista_executing,pcb);

            quitar_de_lista(lista_blocked,pcb);

            quitar_de_lista(lista_suspended_blocked,pcb);

            list_add(lista_exit,pcb);

            return ejecutar_planificador_corto(NULL);

        }

    }

    if(puedo_uno_mas()){

        t_pcb *pcb;

        pcb=list_remove(lista_new,0);

        pcb->estado=READY;

        list_add(lista_ready,pcb);

        log_info(logger,"Carpincho [%d] pasa a estado READY ",pcb->pid);

        return ejecutar_planificador_corto(NULL);

    }

    return ejecutar_planificador_mediano(NULL);

}



void ejecutar_planificador_mediano(t_pcb *pcb){

    if(pcb!=NULL){      //el pcb esta bloqueado-suspendido

        quitar_de_lista(lista_suspended_blocked,pcb);

        list_add(lista_suspended_ready,pcb);

        return(ejecutar_planificador_mediano(NULL));

    }

    if(debo_dessuspender()==1){

        pcb=list_remove(lista_suspended_ready,0);

        pcb->estado=READY;

        log_info(logger,"Carpincho [%d] pasa a estado READY ",pcb->pid);

        //aviso a memoria para que cargue las paginas

        list_add(lista_ready,pcb);

        return ejecutar_planificador_corto(NULL);

    }

    if (debo_suspender()){

        pcb=list_remove(lista_blocked,list_size(lista_blocked)-1);

        //aviso a memoria para que suspenda

        pcb->estado=SUSPENDEDBLOCKED;

        list_add(lista_suspended_blocked,pcb);

        log_info(logger,"Carpincho [%d] pasa a estado SUSPENDIDO BLOQUEADO ",pcb->pid);

        return ejecutar_planificador_largo(NULL);

    }

    return;

}



void ejecutar_planificador_corto(t_pcb *pcb){

    if (pcb!=NULL){

        switch (pcb->estado){

        case EXECUTING:     //estaba ejecutando y se bloqueó

            quitar_de_lista(lista_executing,pcb);

            list_add(lista_blocked,pcb);

            pcb->estado=BLOCKED;

            //pcb->duracionRealAnterior= (clock() / (CLOCKS_PER_SEC/1000) ) - pcb->instanteEstado;

            //pcb->instanteEstado=clock() / (CLOCKS_PER_SEC/1000);

            pcb->duracionRealAnterior= (time(NULL) - pcb->instanteEstado)*1000;

            pcb->instanteEstado=time(NULL) ;

            log_info(logger,"Carpincho [%d] pasa a estado BLOQUEADO",pcb->pid);

            break; 

        default:

            quitar_de_lista(lista_blocked,pcb);

            list_add(lista_ready,pcb);

            pcb->estado=READY;

            log_info(logger,"Carpincho [%d] pasa a estado READY",pcb->pid);

            //pcb->instanteEstado=clock() / (CLOCKS_PER_SEC/1000);

            pcb->instanteEstado=time(NULL) ;

            break;

        }

    }

    if(tengo_procesos()){

        return planificar_algoritmo();

    }

    return ejecutar_planificador_mediano(NULL);

}



void planificar_algoritmo(void){

    if(algoritmo==SJF){return planificar_sjf();}

    return planificar_hrrn();

}





/*

double calculoEstimado (clock_t duracionRealAnterior,clock_t estimacionAnterior)

{

    double alfa = kernel_config->ALFA;

    return (alfa * duracionRealAnterior) + ( (1 - alfa) * estimacionAnterior);

}



clock_t calculoRR (clock_t tiempoEsperaEnReady,clock_t duracionRealAnterior,clock_t estimacionAnterior)

{

    return 1 + (tiempoEsperaEnReady/calculoEstimado(duracionRealAnterior,estimacionAnterior));

}

*/



double calculoEstimado (time_t duracionRealAnterior,time_t estimacionAnterior){

    double alfa = kernel_config->ALFA;

    return (alfa * duracionRealAnterior) + ( (1 - alfa) * estimacionAnterior) ;

}



time_t calculoRR (time_t tiempoEsperaEnReady,time_t duracionRealAnterior,time_t estimacionAnterior){

    return 1 +( ( (time(NULL) - tiempoEsperaEnReady) *1000) /calculoEstimado(duracionRealAnterior,estimacionAnterior) );

}



t_pcb* menorSJF (t_pcb* pcb1,t_pcb* pcb2){

    int calculoEstimado_pcb1 = calculoEstimado(pcb1->duracionRealAnterior,pcb1->estimacionAnterior);

    int calculoEstimado_pcb2 = calculoEstimado(pcb2->duracionRealAnterior,pcb2->estimacionAnterior);

    //log_info(logger,"Comparo pcb1 [%d] y pcb2[%d], estimado1 [%d] y estimado2 [%d] ",pcb1->pid, pcb2->pid,calculoEstimado_pcb1,calculoEstimado_pcb2 );

    if (calculoEstimado_pcb1 <= calculoEstimado_pcb2) return pcb1;

    else return pcb2;

}



t_pcb* mayorRR (t_pcb* pcb1,t_pcb* pcb2){

    int RR_pcb1 = calculoRR(pcb1->instanteEstado,pcb1->duracionRealAnterior,pcb1->estimacionAnterior);

    int RR_pcb2 = calculoRR(pcb2->instanteEstado,pcb2->duracionRealAnterior,pcb2->estimacionAnterior);

    //log_info(logger,"Comparo pcb1 [%d] y pcb2[%d], RR_pcb1 [%d] y RR_pcb2 [%d] ",pcb1->pid, pcb2->pid,RR_pcb1,RR_pcb2 );

    if (RR_pcb1 >= RR_pcb2) return pcb1;

    else return pcb2;

}



void planificar_sjf(void){

    t_pcb* _menorSJF ( void* _pcb1,void* _pcb2) {

        t_pcb* pcb1 = (t_pcb*)_pcb1;

        t_pcb* pcb2 = (t_pcb*)_pcb2;

        return menorSJF(pcb1,pcb2);

    };

    t_pcb* pcb_menorRafaga;

    log_info(logger,"SJF: Hay %d carpinchos listos para ejecutar",list_size(lista_ready));

    pcb_menorRafaga = list_get_minimum(lista_ready,(void*)_menorSJF);

    

    quitar_de_lista(lista_ready,pcb_menorRafaga);

    pcb_menorRafaga->estimacionAnterior = calculoEstimado(pcb_menorRafaga->duracionRealAnterior,pcb_menorRafaga->estimacionAnterior);

    log_info(logger,"SJF: Nuevo estimado anterior [%d] para el carpincho [%d] ",pcb_menorRafaga->estimacionAnterior,pcb_menorRafaga->pid);

    return ejecutar(pcb_menorRafaga);

}





void planificar_hrrn(void){

    t_pcb* _mayorRR ( void* _pcb1,void* _pcb2){

        t_pcb* pcb1 = (t_pcb*)_pcb1;

        t_pcb* pcb2 = (t_pcb*)_pcb2;

        return mayorRR(pcb1,pcb2);

    };

    t_pcb* pcb_mayorRR;

    log_info(logger,"HRRN: Hay %d carpinchos listos para ejecutar",list_size(lista_ready) );

    pcb_mayorRR = list_get_maximum(lista_ready,(void*) _mayorRR);

    

    quitar_de_lista(lista_ready,pcb_mayorRR);

    pcb_mayorRR->estimacionAnterior = calculoEstimado(pcb_mayorRR->duracionRealAnterior,pcb_mayorRR->estimacionAnterior);

    log_info(logger,"HRRN: Nuevo estimado anterior [%d] para el carpincho [%d] ",pcb_mayorRR->estimacionAnterior,pcb_mayorRR->pid);

    return ejecutar(pcb_mayorRR);

}



void ejecutar(t_pcb *pcb){

    int nroCpu=asigno_cpu();

    list_add(lista_executing,pcb);

    pcb->estado=EXECUTING;

    //pcb->estimacionAnterior=estimar(pcb);

    //pcb->instanteEstado =clock() / (CLOCKS_PER_SEC/1000);//marco el momento que incia la ejecucion

    pcb->instanteEstado =time(NULL);//marco el momento que incia la ejecucion

    cpus[nroCpu].pcb=pcb;

    log_info(logger,"Carpincho [%d] pasa a estado EJECUTANDO en la CPU [%d]",pcb->pid, nroCpu);

    sem_post(cpus[nroCpu].mi_semaforo);//doy ok para que inicie

    return;

}



int asigno_cpu(void){

    for (int i=0;i<kernel_config->GRADO_MULTIPROCESAMIENTO;i++){

        if (cpus[i].estado==LIBRE){

            cpus[i].estado=OCUPADO;

            return i;

        }

    }

    // habria que devolver un num negativo quizás para sacar el warning

}



int tengo_procesos(void){

    int gradoP=kernel_config->GRADO_MULTIPROCESAMIENTO;

    int cantReady=list_size(lista_ready);

    int cantExec=list_size(lista_executing);

    if (gradoP>cantExec && cantReady>0){

        return 1;        

    }

    return 0;

}



int debo_dessuspender(void){

    if(puedo_uno_mas()==1 && list_size(lista_suspended_ready)>0){

        return 1;

    }

    return 0;

}



int debo_suspender(void){

/*Que haya carpinchos en BLOCKED

* Que no haya carpinchos en READY

* Que haya carpinchos en NEW

*/

    int cantReady=list_size(lista_ready);

    int cantBloqued=list_size(lista_blocked);

    int cantNew=list_size(lista_new);

    if(cantReady==0&&cantBloqued>0&&cantNew>0){

        return 1;

    }

    return 0;

}



int puedo_uno_mas(void){

    int grado=kernel_config->GRADO_MULTIPROGRAMACION;

    int cantReady=list_size(lista_ready);

    int cantBloqued=list_size(lista_blocked);

    int cantExec=list_size(lista_executing);

    if (grado > (cantReady+cantBloqued+cantExec)){

        return 1;

    }

    return 0;

}



void iniciar_detector_deadlock(void){

    pthread_t hilo;

    pthread_create(&hilo,NULL,(void *)_iniciar_detector_deadlock,0);

	pthread_detach(hilo);

}





///funciones auxiliares



int busco_deadlock(t_pcb *pcb){

    t_semaforo *sem;

	t_pcb *ini = list_get(lista_deadlock,0);    //tomo el primero siempre

	if (pcb==NULL){

	    sem = ini->semaforo_espero;             //el semaforo que esta esperando

	}

	else{                                       //la funcion se llamo recursivamente

		sem=pcb->semaforo_espero;

	}

    if (sem !=NULL){

	for (int j=0;j<list_size(sem->lista_usando);j++){

		pcb=list_get(sem->lista_usando,j);

			if (esperamissemaforos(ini,pcb)==1){

			//!!hay deadlock!!

				list_add(lista_deadlock,pcb);

				return 1;

			}

		if (busco_deadlock(pcb)==1){

			list_add(lista_deadlock,pcb);

			return 1;

		}

	}

    }

	return 0;

}

	

int esperamissemaforos(t_pcb *pcb,t_pcb *pcb2){

    t_semaforo *sem;

	for (int i=0;i<list_size(pcb->lista_semaforos_asignados);i++){

		sem=list_get(pcb->lista_semaforos_asignados,i);

		if (pcb2->semaforo_espero== sem){

			return 1;

		}

	}

	return 0;

}



void* _iniciar_detector_deadlock(void){

    t_pcb *pcb;



    while (1){

		usleep(kernel_config->TIEMPO_DEADLOCK  * 1000);

        sem_wait(sem_lista_semaforos);

		for (int i=0;i<list_size(lista_bloqueados_por_semaforos);i++){

			pcb=list_get(lista_bloqueados_por_semaforos,i);

			list_add(lista_deadlock,pcb);        

			if(busco_deadlock(0)==1){

				aplicarPoliticaDeRecuperacion();

				list_clean(lista_deadlock);

				break;

			}

			list_clean(lista_deadlock);

		}

        sem_post(sem_lista_semaforos);

    }

    return;

}



void aplicarPoliticaDeRecuperacion (){

    int min = 0;

    t_pcb* pcb;

    t_pcb* aux;

    t_semaforo *temp;

    log_info(logger,"-----------> HAY DEADLOCK! ------------");

    for (int i=0;i<list_size(lista_deadlock);i++){

        aux= list_get(lista_deadlock,i);

        log_info(logger,"DEADLOCK: El carpincho [%i] esta en deadlock",aux->pid);

        if (aux->pid > min){

            pcb=aux;

            min=pcb->pid;

        }

    };

    log_info(logger,"------ DEADLOCK: se aplica politica de recuperacion ------");

    log_info(logger,"Se finaliza el carpincho [%d]",pcb->pid);

    sem_wait(sem_planificador);

    enviar_codigo(-9,pcb->socket_carpincho);

    ejecutar_planificador_largo(pcb);

    quitar_de_lista(lista_bloqueados_por_semaforos,pcb);

    pcb->semaforo_espero->valorActual++;

    quitar_de_lista(pcb->semaforo_espero->lista_espera,pcb);

    sem_post(sem_planificador);

    

    for(int j = 0;j<list_size(pcb->lista_semaforos_asignados);j++){  

        temp = list_get(pcb->lista_semaforos_asignados,j);

        quitar_de_lista(temp->lista_usando,pcb);

        quitar_de_lista(temp->lista_espera,pcb);

        aux = list_get(temp->lista_espera,0);

        list_add(temp->lista_usando,aux);

        list_add(aux->lista_semaforos_asignados,temp);

        quitar_de_lista(lista_bloqueados_por_semaforos,aux);

        aux->semaforo_espero=NULL;

        interrupcion_fin_bloqueo(aux);

    }

}







       