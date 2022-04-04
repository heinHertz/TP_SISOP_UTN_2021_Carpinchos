#include "pruebas_swamp.h"

void prueba_test_1(void){   // TEST SOBRE ASIGNACION DE MEMRIA DINAMICA 

    TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '1' ,global_config_swamp->TAMANIO_PAGINA );


        char * buffer2 = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer2, '2' ,global_config_swamp->TAMANIO_PAGINA );

        // t_estructura_administrativa * particion = list_get(listado_particiones, 0 );


        inicializar_proceso_en_el_sistema_particiones(10);
        inicializar_proceso_en_el_sistema_particiones(100);
        inicializar_proceso_en_el_sistema_particiones(20);

        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer2);
        guardar_marco_en_swamp(20, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(20, buffer);

        test_marcos_del_sistema();

        free(buffer);
        free(buffer2);
        sleep(3);
}

void prueba_test_2(void){  // TEST DE PERSISTENCIA SIMPLE  

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

        t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

        persistir_bloque_memoria_marco( buffer , 0,  particion->puntero_mmap  );

        persistir_bloque_memoria_marco( buffer , 2,  particion->puntero_mmap  );


        test_marcos_del_sistema();
        free(buffer);
        sleep(3);
};



void prueba_test_3(void){   // TEST DE BORRADO SIMPLE Y DIRECTO --EL SISTEMA NO REGISTRA--


  TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

        t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

        inicializar_proceso_en_el_sistema_particiones(10);

        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);

        borrar_bloque_memoria_marco(1, particion->puntero_mmap );  //BORRADO DIRECTO

        test_marcos_del_sistema();   //CONFIRMAR CON  EDITOR HEX
        free(buffer);
        sleep(3);
};
void prueba_test_4(void){   //TEST  BORRADO FUNCION  --SE REGISTRA EL BORRADO--
    
  TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

        t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

        inicializar_proceso_en_el_sistema_particiones(10);

        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);

        test_marcos_del_sistema();

        borrar_marco(particion, 0 );
        borrar_marco(particion, 1 );

        test_marcos_del_sistema();   //CONFIRMAR CON  EDITOR HEX
        free(buffer);
        sleep(3);
};

void prueba_test_5(void){   //   borrar_todos_los_marcos_de_un_proceso(uint32_t pid)

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

        inicializar_proceso_en_el_sistema_particiones(10);
        inicializar_proceso_en_el_sistema_particiones(100);

        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);

        test_marcos_del_sistema();

        borrar_todos_los_marcos_de_un_proceso(10);

        test_marcos_del_sistema();
        free(buffer);
        sleep(3);
};
void prueba_test_6(void){


 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

     //   t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

        inicializar_proceso_en_el_sistema_particiones(10);
        inicializar_proceso_en_el_sistema_particiones(100);

        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(10, buffer);

        test_marcos_del_sistema();
        //ok cada marco en su particion
        free(buffer);
        sleep(3);
};
void prueba_test_7(void){   // VAMO CON LAS ASIGNACIONES FIJA 

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

 //   t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

    guardar_marco_en_swamp(10, buffer);

    test_marcos_del_sistema();
    // ok reservo 10 marcos
    free(buffer);
    sleep(3);
};


void prueba_test_8(void){   //ASIGNACION FIJA FUNCIONA PARA  2 PROCESOS

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

    //  t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

    inicializar_proceso_en_el_sistema_particiones(10);
    inicializar_proceso_en_el_sistema_particiones(100);

    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(100, buffer);
    guardar_marco_en_swamp(100, buffer);

    test_marcos_del_sistema();
    // ok reservo 10 marcos
    free(buffer);
    sleep(3);
};
void prueba_test_9(void){

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

  //  t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

    inicializar_proceso_en_el_sistema_particiones(10);
    inicializar_proceso_en_el_sistema_particiones(100);
    inicializar_proceso_en_el_sistema_particiones(20);


    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(100, buffer);
    guardar_marco_en_swamp(100, buffer);
    guardar_marco_en_swamp(20, buffer);
    guardar_marco_en_swamp(20, buffer);

    test_marcos_del_sistema();
    // ok reservo 10 marcos
    free(buffer);
    sleep(3);

};
void prueba_test_10(void){

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '8' ,global_config_swamp->TAMANIO_PAGINA );

    t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

    guardar_marco_en_swamp(10, buffer);
    guardar_marco_en_swamp(10, buffer);

    char * buffer_respuesta = pedir_marco_particion(particion, 0 );
    test_marcos_del_sistema();
    // buffer_respuesta[global_config_swamp->TAMANIO_PAGINA] = '\0';
    printf("buffer:  --%s--  \n", buffer_respuesta);

    free(buffer);
    free(buffer_respuesta);
    sleep(3);
};

//TEST SIMPLE DE EXISTENCIA DE PROCESO EN EL SISTEMA  debe dar 1
void prueba_test_11(void){


 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );
    int pid=10;

    guardar_marco_en_swamp(pid, buffer);

    int resultado = tiene_sistema_swamp_proceso_numero_pid( pid );

    printf("proceso : %d RESULTADO DE TEST  %d", pid , resultado );
    free(buffer);
    sleep(3);
}

//TEST SIMPLE DE EXISTENCIA DE PROCESO EN EL SISTEMA  debe dar 0
void prueba_test_12(void){

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );
    int pid=10;  int pid_diferente=20;

    guardar_marco_en_swamp(pid, buffer);

    int resultado = tiene_sistema_swamp_proceso_numero_pid( pid_diferente );

    printf("proceso : %d RESULTADO DE TEST  %d", pid_diferente , resultado );
    free(buffer);
}

//TEST RESERVAR PROCESO EN EL SISTEMA  DINAMICA
void prueba_test_13(void){

    TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

    inicializar_proceso_en_el_sistema_particiones(10);

    inicializar_proceso_en_el_sistema_particiones(20);
    
    inicializar_proceso_en_el_sistema_particiones(100);

    test_marcos_del_sistema();
    sleep(3);
};


//TEST RESERVAR PROCESO EN EL SISTEMA  FIJA
void prueba_test_14(void){

    TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

    inicializar_proceso_en_el_sistema_particiones(10);

    inicializar_proceso_en_el_sistema_particiones(20);
    
    inicializar_proceso_en_el_sistema_particiones(100);

    test_marcos_del_sistema();
    sleep(3);
};

// TEST 2 PROCESOS
void prueba_test_15(void){

    TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );
    int pid=10;  

    inicializar_proceso_en_el_sistema_particiones(pid);
    inicializar_proceso_en_el_sistema_particiones(20);

    int marco;
    marco = guardar_marco_en_swamp(pid, buffer);

    printf("Pid %d , Marco: %d \n", pid, marco);

    marco = guardar_marco_en_swamp(pid, buffer); //hex ok
    printf("pid: %d marco: %d \n", pid, marco);
    marco = guardar_marco_en_swamp(20, buffer);  //hex ok
    printf("Pid %d , Marco: %d \n", 20, marco);
    test_marcos_del_sistema();
    free(buffer);
    sleep(3);

};

void prueba_test_16(void){

    TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );
    int pid=10;  

    inicializar_proceso_en_el_sistema_particiones(pid);
    inicializar_proceso_en_el_sistema_particiones(20);

    int marco;

    marco = guardar_marco_en_swamp(pid, buffer);
    printf("Pid %d , Marco: %d \n", pid, marco);

    marco = guardar_marco_en_swamp(pid, buffer); //hex ok
    printf("pid: %d marco: %d \n", pid, marco);

    marco = guardar_marco_en_swamp(20, buffer);  //hex ok
    printf("Pid %d , Marco: %d \n", 20, marco);

    test_marcos_del_sistema();
    free(buffer);
    sleep(3);
};

//TEST LEER MARCO
void prueba_test_17(void){

    TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_DINAMICA;

    char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
    memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );
    int pid=10;  

    inicializar_proceso_en_el_sistema_particiones(pid);

    int marco;

    marco = guardar_marco_en_swamp(pid, buffer);
    printf("Pid %d , Marco: %d \n", pid, marco);

    char * salida = leer_marco_proceso(pid, marco);
  
    printf( "RESULTADO LECTURA: ----%s----", salida );
    free(salida);
    free(buffer);
};


//testeo funcion FINALIZAR PROCESO, BORRA TODO LOS PROCESOS EXISTENTES EN EL SISTEMA EN CUALQUIER PARTICION
void prueba_test_18(void){

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '2' ,global_config_swamp->TAMANIO_PAGINA );

    //    t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

    inicializar_proceso_en_el_sistema_particiones(10);
    inicializar_proceso_en_el_sistema_particiones(100);
    inicializar_proceso_en_el_sistema_particiones(20);


        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(20, buffer);
        guardar_marco_en_swamp(20, buffer);
     
        finalizar_proceso(10);
        finalizar_proceso(100);
        finalizar_proceso(20);
     
        test_marcos_del_sistema();
        free(buffer);
        sleep(3);
};

void prueba_test_19(void){

 TIPO_ASIGNACION_MEMORIA = TIPO_ASIGNACION_FIJA;

        char * buffer = malloc(global_config_swamp->TAMANIO_PAGINA);
        memset(buffer, '9' ,global_config_swamp->TAMANIO_PAGINA );

      //  t_estructura_administrativa * particion = list_get(listado_particiones, 0 );

    inicializar_proceso_en_el_sistema_particiones(10);
 //   inicializar_proceso_en_el_sistema_particiones(100);
  //  inicializar_proceso_en_el_sistema_particiones(20);

    // test_marcos_del_sistema();
     sleep(2);

        guardar_marco_en_swamp(10, buffer);
    /*     guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(10, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(100, buffer);
        guardar_marco_en_swamp(20, buffer);
        guardar_marco_en_swamp(20, buffer);
     
        finalizar_proceso(10);
        finalizar_proceso(100);
        finalizar_proceso(20);
     */
      //  test_marcos_del_sistema();
      free(buffer);
        sleep(2);
};


void prueba_test_20(void){


            prueba_test_1( );
            prueba_test_2( );
            prueba_test_3( );
            prueba_test_4( );
            prueba_test_5( );
            prueba_test_6( );
            prueba_test_7( );
            prueba_test_8( );
            prueba_test_9( );
            prueba_test_10( );
            prueba_test_11( );
            prueba_test_12( );
            prueba_test_13( );
            prueba_test_14( );
            prueba_test_15( );
            prueba_test_16( );
            prueba_test_17( );
            prueba_test_18( );
            prueba_test_19( );


};


void prueba_test_21(void){};