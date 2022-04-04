#include "memoria.h"
#include <commons/bitarray.h>
#include <commons/temporal.h>

void iniciar_ram(void){
	log_info(logger,"inicio RAM");
	pid=1;
	instante=1;
	totmiss=0;
	tothit=0;
	//creo la memoria principal y la incializo
	puntero_memoria_principal = malloc(memoria_config->TAMANIO);
	assert(puntero_memoria_principal !=NULL);
	for (int i=0;i<memoria_config->TAMANIO;i++){
		puntero_memoria_principal[i]='a';
	}
	tamHeap = sizeof(t_alloc);
	

    //Creo birarray -> Indica frames libres 2048bytes/64bytes= 32Paginas
    tamanioBitMapPrincipal = (memoria_config->TAMANIO/memoria_config->TAMANIO_PAGINA);
	cantFrames = tamanioBitMapPrincipal;
	mapaDeBits=malloc(sizeof(int)*cantFrames);
	assert(mapaDeBits!=NULL);
	for(int i=0;i<cantFrames;i++){
		mapaDeBits[i]=0;
	}
	
	//creo la tlb y la inicializo
	TLB = malloc(sizeof(t_tlb)* memoria_config->CANTIDAD_ENTRADAS_TLB);
	assert(TLB !=NULL);
	for(int i=0;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
		TLB[i].cont=0;
		TLB[i].frame=0;
		TLB[i].pid=0;
		TLB[i].pagina=0;
	}
	sem_init(&sem_list,0,1);
	sem_init(&sem_tlb,0,1);
	sem_init(&sem_bitmap,0,1);
	sem_init(&sem_ram,0,1);
	sem_init(&sem_tablas,0,1);
	sem_init(&sem_swamp,0,1);
	sem_init(&sem_fallo,0,1);
	sem_init(&sem_inst,0,1);
	listaPCB=list_create();
	frames = malloc(sizeof(frames)*cantFrames);
	listaContadores=list_create();
	if (strcmp(memoria_config->ALGORITMO_REEMPLAZO_MMU,"CLOCK-M") == 0){
		metodo_reemplazo_mmu = CLOCKM;
		log_info(logger,"reemplazo MMU es CLOCK MODIFICADO");
	}
	else {
		metodo_reemplazo_mmu = LRU;
		log_info(logger,"reemplazo MMU es LRU");
	}
	if (strcmp(memoria_config->ALGORITMO_REEMPLAZO_TLB,"FIFO") == 0){
		metodo_reemplazo_tlb = FIFO;
		log_info(logger,"reemplazo TLB es FIFO");
	}
	else {
		metodo_reemplazo_tlb = LRU;
		log_info(logger,"reemplazo TLB es LRU");
	}
		if (strcmp(memoria_config->TIPO_ASIGNACION,"DINAMICA") == 0){
		metodo_asignacion = DINAMICA;
		memoria_config->MARCOS_POR_CARPINCHO=cantFrames;
		
		log_info(logger,"asignacion es DINAMICA");
	}
	else {
		metodo_asignacion = FIJA;
		log_info(logger,"asignacion es FIJA");
	}

}

void suspender_proceso(t_pcb *pcb){
	if (pcb->alocado==0){return;}
	if (metodo_asignacion==FIJA){
		sem_wait(&sem_bitmap);
		for (int i=0;i<memoria_config->MARCOS_POR_CARPINCHO;i++){
			mapaDeBits[pcb->frames[i]->frame]=LIBRE;
			enviar_victima(pcb,pcb->frames[i]->indice);
			pcb->frames[i]->valido=0;
			pcb->frames[i]=NULL;
		}
		sem_post(&sem_bitmap);
		return;
	}
	sem_wait(&sem_bitmap);
	for(int i=0;i<= pcb->alocado /memoria_config->TAMANIO_PAGINA;i++){
		if (pcb->tabla_paginas[i].valido==1){
			mapaDeBits[pcb->tabla_paginas[i].frame]=LIBRE;
			enviar_victima(pcb,i);
			pcb->tabla_paginas[i].valido=0;
		}
	}
	sem_post(&sem_bitmap);
	return;
}

uint32_t buscar_hueco (uint32_t size, t_pcb *PCB){
	log_info(logger,"intentare alocar %i bytes para el proceso",size,PCB->pid);
	t_alloc heap;
	uint32_t direccion_retorno=0;
	uint32_t dirheap =0;
	//uint32_t dirheapprev =0;
	int tamheap = sizeof(t_alloc);
	int encontrado;
	if (PCB->alocado > 0){
		leer(&heap,tamheap,dirheap,PCB);
		encontrado = cabe(size,dirheap,heap);
		while ((encontrado < 0) && (heap.nextAlloc != NULL) ){
			//dirheapprev =dirheap;
			dirheap = heap.nextAlloc;
			leer(&heap,tamheap,dirheap,PCB);
			encontrado = cabe(size,dirheap,heap);
		}
	}
	else {
		log_info(logger,"es el primer alloc del proceso %i",PCB->pid);
		if(metodo_asignacion==FIJA && (size + tamheap*2) > paginasMaximas * memoria_config->TAMANIO_PAGINA){return -2;}
		encontrado = primer_alloc(PCB);
		heap.nextAlloc =NULL;
		heap.prevAlloc =NULL;
		heap.isFree=1;
		
		}
	
	

	switch (encontrado){
		case -2:
			//no puedo iniciar un hueco
			direccion_retorno =0;
			break;
		case -1:
			//no hay hueco
			direccion_retorno = nuevo_alloc(size,heap,dirheap,PCB);
			break;
		case 0:
			//entra justo, lo marco como libre y guardo
			direccion_retorno = dirheap + tamheap;
			heap.isFree = 0;
			escribir(&heap,tamheap,dirheap,PCB);
			break;
		case 1:
			//entra y sobra lugar
			direccion_retorno = dirheap + tamheap;
			uint32_t next = heap.nextAlloc; //guardo direccion del next
			heap.isFree = 0;
			heap.nextAlloc = dirheap+tamheap+size; 
			escribir(&heap,tamheap,dirheap,PCB);
			//creo un nuevo heap
			heap.isFree=1;
			heap.prevAlloc=dirheap;
			heap.nextAlloc=next;
			escribir(&heap,tamheap,dirheap+tamheap+size,PCB);

			leer(&heap,tamheap,next,PCB);
			heap.prevAlloc = dirheap+tamheap+size;
			
			escribir(&heap,tamheap,next,PCB);

			break;
	}

	return direccion_retorno;
}

int primer_alloc(t_pcb *PCB){
	t_alloc heap;
	heap.isFree=1;
	heap.nextAlloc=0;
	heap.prevAlloc=0;
	
	int asign=asignarPaginas(sizeof(t_alloc)/memoria_config->TAMANIO_PAGINA+1,PCB);
	if (asign < 0){return -2;}
	PCB->alocado=sizeof(t_alloc);
	escribir(&heap,sizeof(t_alloc),0,PCB);
	
	return -1;
}
uint32_t nuevo_alloc(uint32_t size, t_alloc heap,uint32_t dirheap,t_pcb *PCB){
	uint32_t tamheap = sizeof(t_alloc);
	uint32_t direccion = tamheap + dirheap;
	int ocupa = tamheap + size;
	uint32_t cant_frames_necesarios = 0;
	int librePag = memoria_config->TAMANIO_PAGINA - (PCB->alocado%memoria_config->TAMANIO_PAGINA);//cuanto me queda en la ultima pagina
	if (PCB->alocado%memoria_config->TAMANIO_PAGINA ==0){librePag=0;}
	if (librePag < ocupa){
		cant_frames_necesarios = 1+(ocupa-librePag)/memoria_config->TAMANIO_PAGINA;
		if (asignarPaginas(cant_frames_necesarios,PCB) < 0 ){
			return 0;
		}
	} //no necesito asignar lugar porque entra en la ultima pagina
	heap.isFree = 0;
	heap.nextAlloc = dirheap+ocupa;
	PCB->alocado = PCB->alocado+ocupa;
	
	escribir(&heap,sizeof(t_alloc),dirheap,PCB);
	heap.isFree =1;
	heap.nextAlloc = NULL;
	heap.prevAlloc = dirheap;
	escribir(&heap,sizeof(t_alloc),dirheap+ocupa,PCB);
	
	return direccion;
}

int asignarPaginas(u_int32_t cant,t_pcb *PCB){
	log_info(logger,"intento asignar %i paginas al proceso %i",cant,PCB->pid);
	
	if (metodo_asignacion==FIJA){
		return asignarPaginas_fijo(cant,PCB);
	}
	return asignarPaginas_dinamico( cant,PCB);
}

int asignarPaginas_fijo(u_int32_t cant,t_pcb *PCB){
	int pos;
	if(PCB->alocado ==0){//primer aloc, busco los frameslibres necesarios
		if (reservar_frames_swap(paginasMaximas,PCB)<0){
			return -1;
		};
		int encontrado=0;
		int j=0;
		log_info(logger,"voy a reservar en memoria los %i frames inciales",memoria_config->MARCOS_POR_CARPINCHO);
		sem_wait(&sem_bitmap);
		int pos=0;
		int *tempbit=malloc(sizeof(uint32_t)*memoria_config->MARCOS_POR_CARPINCHO);
		for(j;j<cantFrames && encontrado < memoria_config->MARCOS_POR_CARPINCHO;j++){
			if(mapaDeBits[j]==LIBRE) {
				tempbit[encontrado]=j;
				encontrado++;
			}
		}
		if (encontrado<memoria_config->MARCOS_POR_CARPINCHO){
			sem_post(&sem_bitmap);
			free(tempbit);
			log_info(logger,"no encontre frames libres, no puedo iniciar el carpincho");
			return MATE_WRITE_FAULT;
		}

		PCB->tabla_paginas = malloc(sizeof(t_tabla_paginas)*paginasMaximas);
		PCB->frames=malloc(sizeof(PCB->frames) * memoria_config->MARCOS_POR_CARPINCHO);
		for(int j=0;j<memoria_config->MARCOS_POR_CARPINCHO;j++){
			int i =tempbit[j];
			mapaDeBits[tempbit[j]]=1;
			PCB->tabla_paginas[j].indice=j;
			PCB->tabla_paginas[j].pcb=PCB;
			PCB->tabla_paginas[j].pcb=PCB;
			PCB->tabla_paginas[j].accedido=0;
			PCB->tabla_paginas[j].valido=1;
			PCB->tabla_paginas[j].frame=tempbit[j];
			PCB->tabla_paginas[j].Modificado=0;
			t_tabla_paginas *temp=&PCB->tabla_paginas[j];
			PCB->frames[j] = &PCB->tabla_paginas[j];
			frames[tempbit[j]]=temp;
		}
		sem_post(&sem_bitmap);
		free(tempbit);
		for(int j=memoria_config->MARCOS_POR_CARPINCHO;j<paginasMaximas;j++){
			PCB->tabla_paginas[j].indice=j;
			PCB->tabla_paginas[j].pcb=PCB;
			PCB->tabla_paginas[j].valido=0;
		}
		
		return 0;
	}
	if(PCB->alocado/memoria_config->TAMANIO_PAGINA + 1 + cant > paginasMaximas){
		log_info(logger,"el carpincho %i quiso asignar mas memoria de la que puede",PCB->pid);
		return -1;
	}
	return 0 ;
}
int asignarPaginas_dinamico(u_int32_t cant,t_pcb *PCB){
	//reservo en swamp
		if (reservar_frames_swap(cant,PCB)<0){
			log_info(logram,"no hay lugar en swamp-no reservo nada");
			return -1;
		}
	if (cant == 0){return 1;}
	t_tabla_paginas *temp= malloc(sizeof(t_tabla_paginas)*cant);
	uint32_t encontrados =0;
	sem_wait(&sem_bitmap);
	for (uint32_t i=0;(i<cantFrames)&& (encontrados<cant);i++){
		//if (bitarray_test_bit(bitMap,i)==0){
		if (mapaDeBits[i]==LIBRE){	
			temp[encontrados].frame =i;
			temp[encontrados].valido =1;
			//temp[encontrados].accedido =0;
			encontrados++;
			mapaDeBits[i]=OCUPADO; //bitarray_set_bit(bitMap,i);
			
			log_info(logger,"encontre el frame %i libre",i);
		}
	}
	
	for (encontrados;encontrados<cant;encontrados++){
		temp[encontrados].valido=0;
	}
	log_info(logger,"reserve la memoria necesaria");
	int cantAnt=PCB->paginas;

	int tamNew = sizeof(t_tabla_paginas)*(cantAnt +cant);
	t_tabla_paginas *new = malloc(tamNew);
	
	for (int i=cantAnt;i<cantAnt+cant;i++){//copio los valores de la tabla existente
		new[i].pcb=PCB;
		new[i].indice=i;
		new[i].frame = temp[i-cantAnt].frame;
		new[i].valido = temp[i-cantAnt].valido;
		new[i].accedido = temp[i-cantAnt].accedido;
		new[i].Modificado=temp[i-cantAnt].Modificado;
		if (new[i].valido==1){
			frames[new[i].frame]=&new[i];
		}
	}
	sem_wait(&sem_tablas);
	for (int i=0;i<cantAnt;i++){//copio los valores de la tabla existente
		new[i].frame = PCB->tabla_paginas[i].frame;
		new[i].indice=i;
		new[i].valido = PCB->tabla_paginas[i].valido;
		new[i].accedido = PCB->tabla_paginas[i].accedido;
		new[i].Modificado=PCB->tabla_paginas[i].Modificado;
		new[i].pcb=PCB;
		if (new[i].valido==1){
			frames[new[i].frame]=&new[i];
		}
	}	
	free(PCB->tabla_paginas);
	PCB->tabla_paginas=new;
	sem_post(&sem_bitmap);
	sem_post(&sem_tablas);
	free(temp);
	PCB->paginas=PCB->paginas+cant;
	return 0;

}




int cabe(uint32_t size,uint32_t dirheap,t_alloc heap){
	if (heap.isFree == 0){return -1;}// esta ocupado, chau
	if (heap.nextAlloc==NULL){return -1;}//es el ultimo heap
	int tamheap = sizeof(t_alloc);
	uint32_t tamalloc = (heap.nextAlloc - (dirheap + tamheap));// - 2*dirheap - tamheap;
	if (size == tamalloc){//entra justo, joya
		return 0;
	}
	if (size + tamheap< tamalloc){//entra y sobra espacio para un heap nuevo + 1 byte
	return 1;
	}
	return -1;//no hay espacio
}

int validar_escribir(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB){
	if (dirLogica >= tamHeap && (dirLogica+tamanio) <= PCB->alocado - tamHeap){
		return escribir(valor,tamanio,dirLogica,PCB);
	}
	return MATE_WRITE_FAULT;
	///esto no iria
	/*
	if (PCB->alocado==0|| dirLogica < sizeof(t_alloc)||dirLogica>PCB->alocado-sizeof(t_alloc)){return MATE_WRITE_FAULT;}
	int posicion = 0;
	t_alloc heap;
	//busco el heap al que corresponde
	posicion=leer(&heap,tamHeap,posicion,PCB);
	if (heap.isFree==0 && posicion<=dirLogica && dirLogica+tamanio <= heap.nextAlloc){
		return escribir(valor,tamanio,dirLogica,PCB);
	}
	//posicion=heap.nextAlloc;
	while (heap.nextAlloc>0 && posicion <= dirLogica){//la direccion no es el inicio del hueco, leo el siguiente heap
		posicion=heap.nextAlloc;
		posicion=leer(&heap,tamHeap,posicion,PCB);
		if (heap.isFree==0 && posicion <= dirLogica && dirLogica+tamanio <= heap.nextAlloc){
			return escribir(valor,tamanio,dirLogica,PCB);
		}
	}
	
	if (heap.isFree==0 && (heap.prevAlloc+tamHeap<dirLogica ||dirLogica+tamanio < posicion-tamHeap)){
		return escribir(valor,tamanio,dirLogica,PCB);
	}

	return MATE_WRITE_FAULT;
		*/
}

int escribir(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB){
	//valido datos recibidos
	int posValor =0;
	uint32_t pvalor=0;
	uint32_t frame;
	uint32_t tamPagina=memoria_config->TAMANIO_PAGINA;
	uint32_t despl;
	uint32_t tamEsc;
	uint32_t dirfinal =dirLogica+tamanio;
	int pag_actual=dirLogica/tamPagina;
	int pag_ant;
	int i=0;
	while(dirLogica < dirfinal){		
		pag_ant=pag_actual;
		frame= obtener_frame(pag_actual,PCB);
		PCB->tabla_paginas[pag_actual].Modificado=1;
		log_info(logger,"la pagina %i del carpincho %i fue modificada",pag_actual,PCB->pid);
		while(dirLogica < dirfinal && pag_actual==pag_ant){			
			despl=dirLogica % tamPagina;
			puntero_memoria_principal[frame * tamPagina + despl]=valor[i];
			dirLogica++;
			i++;
			pag_actual=dirLogica/tamPagina;
		}
	}
	return dirLogica;
/*
	for(int pagina= dirLogica/tamPagina;pagina <= dirfinal/tamPagina;pagina++){
		frame = obtener_frame(pagina,PCB);
		despl=dirLogica % tamPagina;
		tamEsc = tamPagina-despl;
		memcpy(&puntero_memoria_principal[frame * tamPagina + despl],&valor[pvalor],tamEsc);
		dirLogica=dirLogica + tamEsc;
		pvalor=pvalor+tamEsc;
		PCB->tabla_paginas[pagina].Modificado=1;
	}
*/

	
	/*
	int posValor =0;
	uint32_t i;
	//log_info(logger,"escribo %i bytes desde la posicion logica %i",tamanio,dirLogica);
	for (i=0;i<tamanio;i++){

		puntero_memoria_principal[direccionFisica(dirLogica,PCB)]=valor[posValor];

		posValor++;
		dirLogica++;

	}
	//semfram
	return dirLogica;
	*/
}

int validar_leer(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB){
	if (dirLogica+tamanio<=PCB->alocado){
		return leer(valor,tamanio,dirLogica,PCB);
	}
	return MATE_READ_FAULT;
};

int leer(char *valor,int tamanio,uint32_t dirLogica, t_pcb *PCB){
	//valido datos recibidos
	int posValor =0;
	uint32_t pvalor=0;
	uint32_t frame;
	uint32_t tamPagina=memoria_config->TAMANIO_PAGINA;
	uint32_t despl;
	
	uint32_t tamLeer;
	uint32_t dirfinal =dirLogica+tamanio;
		int pag_actual=dirLogica/tamPagina;
	int pag_ant=pag_actual;
	int i=0;
	while(dirLogica < dirfinal){
		pag_ant=pag_actual;
		frame= obtener_frame(pag_actual,PCB);
		while(dirLogica < dirfinal && pag_actual==pag_ant){
			despl=dirLogica % tamPagina;
			valor[i]=puntero_memoria_principal[frame * tamPagina + despl];
			dirLogica++;
			i++;
			pag_actual=dirLogica/tamPagina;
		}
	}
	return dirLogica;
	/*
	for(int pagina= dirLogica/tamPagina;pagina <= dirfinal/tamPagina;pagina++){
		frame = obtener_frame(pagina,PCB);
		despl=dirLogica % tamPagina;
		tamLeer = tamPagina-despl;
		memcpy(&valor[pvalor],&puntero_memoria_principal[frame * tamPagina + despl],tamLeer);
		pvalor=pvalor+tamLeer;
		dirLogica=dirLogica + tamLeer;
	}
	*/
}
uint32_t obtener_frame(uint32_t pagina,t_pcb *pcb){
	uint32_t frame;
	sem_wait(&sem_tlb);
	int res=busca_en_TLB(pagina,&frame,pcb);
	sem_post(&sem_tlb);
	if (res == -1){//fue un miss
		frame = busca_en_tabla(pagina,pcb);
		//log_info(logger,"estado previo de TLB");
		sem_wait(&sem_tlb);
		//logTLB();
		agrego_a_TLB(pagina,frame,pcb);
		log_info(logram,"estado posterior de TLB:");
		//logTLB();
		sem_post(&sem_tlb);
		
	}
	pcb->tabla_paginas[pagina].accedido=inc_inst();
	log_info(logger,"la pagina %i del carpincho %i fue accedida en %u",pagina,pcb->pid,pcb->tabla_paginas[pagina].accedido);
	return frame;
}

void logTLB (void){
	char *text[200];
	for (int i=0;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
		if(TLB[i].pid==0){
			sprintf(text,"Entrada:%-4i Estado:Libre    Carpincho: -      Pagina: - Marco: -",i);
		}
		else{
			sprintf(text,"Entrada:%-4i Estado:Ocupado  Carpincho: %-4i     Pagina: %-4i Marco: %-4i",i,TLB[i].pid,TLB[i].pagina,TLB[i].frame);
		}
		log_info(logram,"%s",text);
	}
}

uint32_t direccionFisica(uint32_t dirLogica,t_pcb *PCB){
	t_tabla_paginas *tabla = PCB->tabla_paginas;
	uint32_t nroPagina = dirLogica/memoria_config->TAMANIO_PAGINA;
	uint32_t frame;
	uint32_t despl=dirLogica%memoria_config->TAMANIO_PAGINA;
	uint32_t dirFisica;
	//semwram
	int res=busca_en_TLB(dirLogica,&frame,PCB);
	if (res == -1){//fue un miss
		frame = busca_en_tabla(dirLogica,PCB);
		agrego_a_TLB(nroPagina,frame,PCB);
	}

	dirFisica = frame * memoria_config->TAMANIO_PAGINA + despl;
	tabla[nroPagina].accedido = inc_inst();
	log_info(logger,"accedo a pid %i dirLogica %i dirfisica %i",PCB->pid,dirLogica,dirFisica);
	//log_info(logger,"%i",dirFisica);
		return dirFisica;

	
}





uint32_t busca_en_tabla(uint32_t nroPagina,t_pcb *PCB){
	//devuelve el frame
	t_tabla_paginas *tabla = PCB->tabla_paginas;
	//log_info(logger,"voy a acceder a la pagina  %i del programa en el frame %i",nroPagina,tabla[nroPagina].frame);
	sem_wait(&sem_tablas);
	if (tabla[nroPagina].valido == 0){ //esta en swap
		sem_post(&sem_tablas);
		log_info(logger,"fallo de pagina del carpincho %i",PCB->pid);
		sem_wait(&sem_fallo);
		log_info(logger,"estado previo de frames:");
		//logFallo();
		tabla[nroPagina].frame = traer(nroPagina,PCB);
		tabla[nroPagina].valido = 1;
		tabla[nroPagina].Modificado=0;
		tabla[nroPagina].accedido = inc_inst();
		frames[tabla[nroPagina].frame]=&tabla[nroPagina];
		
		
		log_info(logger,"se resuelve el fallo ahora el frame es %i",tabla[nroPagina].frame);
		log_info(logger,"estado posterior de frames:");
		//logFallo();
		sem_post(&sem_fallo);
		sem_wait(&sem_tablas);
		//frame[tabla[nroPagina].pagina]=&tabla[nroPagina];
	}
	tabla[nroPagina].accedido = inc_inst();
	sem_post(&sem_tablas);
	return tabla[nroPagina].frame;
}
void logFallo(void){
	t_tabla_paginas *tbl;
	t_pcb *pcb;
	
		for (int i=0;i<cantFrames;i++){
			tbl = frames[i];
			if (mapaDeBits[i]==OCUPADO){
			pcb = tbl->pcb;
			log_info(logger,"frame %i  pid:%i pagina:%i accedido:%i modificado:%i",i,pcb->pid,tbl->indice,tbl->accedido,tbl->Modificado);
			}
			else {
				log_info(logger,"frame %i Libre",i);
			}
		}
		return;
	
}
u_long inc_inst (void){
	sem_wait(&sem_inst);
	instante++;
	u_long tmp = instante;
	sem_post(&sem_inst);
	return tmp;
} 
uint32_t traer (uint32_t nroPagina,t_pcb *PCB) {

	int libre=0 ;
	uint32_t frame =buscar_victima_tabla(nroPagina,PCB);

	char *contenidoSwap=traer_pagina(nroPagina,PCB);	
	memcpy(&puntero_memoria_principal[frame * memoria_config->TAMANIO_PAGINA],contenidoSwap,memoria_config->TAMANIO_PAGINA);
	free(contenidoSwap);


	return frame;

}

uint32_t buscar_victima_tabla(int nroPagina,t_pcb *PCB){
	if (metodo_asignacion==DINAMICA){
		return buscar_victima_tabla_dinamico(nroPagina, PCB);
	}
	else {
		return buscar_victima_tabla_fijo(nroPagina, PCB);
	}
}

uint32_t buscar_victima_tabla_fijo(int nroPagina,t_pcb *PCB){
	int frame;
	for (int j=0;j<memoria_config->MARCOS_POR_CARPINCHO;j++){
		if(PCB->frames[j]==NULL){//esta suspendido
			sem_wait(&sem_bitmap);
			for(frame=0;frame<cantFrames;frame++){
				if(mapaDeBits[frame]==LIBRE){
					mapaDeBits[frame]==OCUPADO;
					break;
				}
			}	
			sem_post(&sem_bitmap);
		PCB->frames[j]=&PCB->tabla_paginas[nroPagina];
		return frame;
		}
	}
	if (metodo_reemplazo_mmu == LRU){
		return buscar_victima_tabla_fijo_algoritmo_LRU(nroPagina,PCB);
	}
	return buscar_victima_tabla_fijo_algoritmo_clockM(nroPagina,PCB);
}


uint32_t buscar_victima_tabla_fijo_algoritmo_clockM(int nroPagina,t_pcb *PCB){
	log_info(logger,"busco victima (fijo-clock-M) para el carpincho %i",PCB->pid);
	
	int index;
	for (PCB->clock;PCB->clock<memoria_config->MARCOS_POR_CARPINCHO;PCB->clock++){
		if(PCB->inicioVuelta==PCB->clock){//marco la vuelta
			if (PCB->vuelta !=2){
				PCB->vuelta++;
			}
			else{PCB->vuelta=0;}
		}
		if ((PCB->frames[PCB->clock]->accedido==0 && PCB->frames[PCB->clock]->Modificado == 0)
			|| 
			(PCB->frames[PCB->clock]->accedido == 0 && PCB->frames[PCB->clock]->Modificado == 1 && PCB->vuelta ==2)){
				log_info(logger,"la victima fijo es la pagina %i",PCB->frames[PCB->clock]->indice);
				PCB->frames[PCB->clock]->valido=0;
				//borrar_De_TLB(PCB->pid,PCB->frames[PCB->clock]->indice);
				enviar_victima(PCB,PCB->frames[PCB->clock]->indice);
				PCB->clock++;
				PCB->inicioVuelta=PCB->clock;
				if(PCB->inicioVuelta==memoria_config->MARCOS_POR_CARPINCHO){
					PCB->inicioVuelta=0;
				}
				PCB->vuelta=0;
				t_tabla_paginas *victima=PCB->frames[PCB->clock-1];
				victima->valido=0;
				t_tabla_paginas *tmp=&PCB->tabla_paginas[nroPagina];
				PCB->frames[PCB->clock -1]=&PCB->tabla_paginas[nroPagina];
				return victima->frame;
			}
			else {PCB->frames[PCB->clock]->accedido=0;}
		}
		
	//llegue al final y no encontre, vuelvo a 0 el clock y vuelvo
	PCB->clock=0;
	return buscar_victima_tabla_fijo_algoritmo_clockM(nroPagina,PCB);

}

uint32_t buscar_victima_tabla_dinamico(int nroPagina,t_pcb *PCB){
	//reviso si hay un frame libre en toda la memoria
	//victima=NULL;
	sem_wait(&sem_bitmap);
	for (uint32_t i=0;i<cantFrames;i++){
		//if (bitarray_test_bit(bitMap,i)==0){//encontre un frame libre
		if (mapaDeBits[i]==0){//encontre un frame libre
			mapaDeBits[i]=1;//bitarray_set_bit(bitMap,i);
			sem_post(&sem_bitmap);
			return i;
		}
	}
	sem_post(&sem_bitmap);
	//no hay frame libre, tengo que reemplazar
	if (metodo_reemplazo_mmu == LRU){
		return buscar_victima_tabla_dinamico_algoritmo_LRU(nroPagina,PCB);
	}
	else{
		//aca va semaforo
		return buscar_victima_tabla_dinamico_algoritmo_clockM(nroPagina,PCB);
	}
}
uint32_t buscar_victima_tabla_dinamico_algoritmo_clockM(int nroPagina,t_pcb *PCB){
	log_info(logger,"busco victima (dinamico-clock-M) para el carpincho %i",PCB->pid);
	
	static int vuelta=0;
	static int inicioVuelta =0;
	static uint32_t aguja=0;
	log_info(logger,"la aguja esta en %i",aguja);
	for (aguja;aguja<cantFrames;aguja++){
		if(inicioVuelta==aguja){
			if (vuelta !=2){
				vuelta++;
			}
			else{vuelta=0;}
		}
		sem_wait(&sem_tablas);		
		if ((frames[aguja]->accedido==0 && frames[aguja]->Modificado == 0)
			|| 
			(frames[aguja]->accedido == 0 && frames[aguja]->Modificado == 1 && vuelta ==2)){
			t_pcb *pcbVictima=frames[aguja]->pcb;
			t_tabla_paginas *victima = frames[aguja];
			//borrar_De_TLB(pcbVictima->pid,victima->indice);
			log_info(logger,"encontre victima Dinamico/clock pid:%i, pagina %i, frame %i",pcbVictima->pid,victima->indice,aguja);
			victima->valido=0;
			sem_post(&sem_tablas);
			sem_post(&sem_list);
			t_tabla_paginas *tmp=&PCB->tabla_paginas[nroPagina];
			frames[aguja]=&PCB->tabla_paginas[nroPagina];
			enviar_victima(pcbVictima,victima->indice);
			aguja++;
			vuelta=0;
			inicioVuelta=aguja;
			if (aguja==cantFrames){
				aguja=0;
				inicioVuelta=0;
				return cantFrames-1;
			}
			return aguja-1;
		}
		if (frames[aguja]->accedido > 0 && vuelta == 2){
		frames[aguja]->accedido=0;
		}//lo marco para la proxima vuelta
		
		sem_post(&sem_tablas);
	}

	
	
	//llegue al final y no encontre victima, vuelvo a buscar
	aguja=0;
	
	return buscar_victima_tabla_dinamico_algoritmo_clockM(nroPagina,PCB);
}

uint32_t buscar_victima_tabla_fijo_algoritmo_LRU(int nroPagina,t_pcb *PCB){
	log_info(logger,"busco victima (fijo-LRU) para el carpincho %i",PCB->pid);
	t_tabla_paginas *victima;
	u_long min=inc_inst();
	int pagina=0;
	int cant=PCB->alocado/memoria_config->TAMANIO_PAGINA;
	int pagVictima;
	int ind;
	t_tabla_paginas *aux;
	for (int i=0;i<memoria_config->MARCOS_POR_CARPINCHO;i++){
		log_info(logger,"la pagina %i tiene valor accedido %i, el menor es %i",i,PCB->frames[i]->accedido,min);
		if(PCB->frames[i]->accedido < min){
			victima=PCB->frames[i];
			min=victima->accedido;
			ind =i;
			aux=&PCB->frames[i];
		}
	}
	log_info(logger,"la victima es la pagina %i",ind);
	t_tabla_paginas *temp=&PCB->tabla_paginas[nroPagina];
	PCB->frames[ind]=&PCB->tabla_paginas[nroPagina];

	pagVictima=victima->indice;
	enviar_victima(PCB,pagVictima);	
	//fin, encontre la victima
	victima->valido=0;
	return victima->frame;
}

uint32_t buscar_victima_tabla_dinamico_algoritmo_LRU(int nroPagina,t_pcb *PCB){
	log_info(logger,"busco victima (dinamico-LRU) para el carpincho %i",PCB->pid);
	t_pcb *pcbTemp;
	t_tabla_paginas *victima;
	u_long min=inc_inst();
	int pagina=0;
	int cant;
	int indice=0;
	sem_wait(&sem_list);
	sem_wait(&sem_tablas);
	int pagVictima;
	t_pcb *pcbVictima;
	int indx;
	for(int i=0;i<cantFrames;i++){
		log_info(logger,"el frame %i tiene acceso %i",i,frames[i]->accedido);
		if (frames[i]->accedido < min){
			min=frames[i]->accedido;
			pcbVictima=frames[i]->pcb;
			victima=frames[i];
			pagVictima=victima->indice;
			indx=i;
		}
	}
	t_tabla_paginas *tmp=&PCB->tabla_paginas[nroPagina];
	frames[indx]=&PCB->tabla_paginas[nroPagina];
	

	
	victima->valido=0;
	sem_post(&sem_list);
	sem_post(&sem_tablas);
	//fin, encontre la victima
	enviar_victima(pcbVictima,pagVictima);
	return victima->frame;
}


void enviar_victima(t_pcb *PCB,uint32_t pagVictima){
	borrar_De_TLB(PCB->pid,pagVictima);
	if (PCB->tabla_paginas[pagVictima].Modificado==0){
		log_info(logger,"la victima no fue modificada, no envio a swamp");
		return;
		}
	log_info(logger,"la victima fue modificada, la envio a swamp");
	char *datosRam = malloc(memoria_config->TAMANIO_PAGINA);
	int frame = PCB->tabla_paginas[pagVictima].frame;
	//cargo en datosRam el contenido de memoria de la victima
	memcpy(datosRam,&puntero_memoria_principal[frame*memoria_config->TAMANIO_PAGINA],memoria_config->TAMANIO_PAGINA);
	enviar_pagina(datosRam,pagVictima, PCB);
	free(datosRam);
	
}

int busca_en_TLB(uint32_t paginaBuscada,uint32_t *frame,t_pcb *PCB){
	uint32_t pidBuscado = PCB->pid;
	

	for(int i=0;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
		if(TLB[i].pid == pidBuscado && TLB[i].pagina == paginaBuscada){
			log_info(logram,"HIT de pid:%i, pagina:%i, frame %i",pidBuscado,paginaBuscada,TLB[i].frame);
			if (metodo_reemplazo_tlb == LRU){//actualizo el momento de acceso de la TLB
				TLB[i].cont=inc_inst();
				
			}
			nuevo_hit(pidBuscado);
			*frame = TLB[i].frame;
		//	sem_post(&sem_tlb);
			return 0;
		}
	}
	log_info(logram,"MISS de pid:%i, pagina:%i",pidBuscado,paginaBuscada);
	nuevo_miss(pidBuscado);
	
	
return -1;
}

void borrar_De_TLB(uint32_t pid,uint32_t pagina){
	sem_wait(&sem_tlb);
	for(int i=0;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
		if(TLB[i].pid == pid && TLB[i].pagina == pagina){
			log_info(logger,"borro de tlb la pagina");
			TLB[i].pid=0;
			TLB[i].pagina=0;
			TLB[i].cont=0;
			sem_post(&sem_tlb);
			return;
		}
	}
	sem_post(&sem_tlb);
	return;
}

void nuevo_miss(u_int32_t pidBuscado){
	t_contadores *cont = buscar_contador(pidBuscado);
	cont->count_tlb_miss++;
	totmiss++;
	usleep(memoria_config->RETARDO_FALLO_TLB *1000);
	return;
}

void nuevo_hit(u_int32_t pidBuscado){
	t_contadores *cont = buscar_contador(pidBuscado);
	cont->count_tlb_hit++;
	tothit++;
	usleep(memoria_config->RETARDO_ACIERTO_TLB *1000);
	return;
}

void *buscar_contador(u_int32_t pid){
	t_contadores *cont;
	for (int i=0;i<listaContadores->elements_count;i++){
		cont=list_get(listaContadores,i);
		if (cont->pid==pid){return cont;}
	}
	return NULL;
}




void agrego_a_TLB(uint32_t pagina,uint32_t frame,t_pcb *PCB){
	uint32_t pos = buscar_victima_tlb();
	TLB[pos].pid=PCB->pid;
	TLB[pos].frame=frame;
	TLB[pos].pagina=pagina;
	TLB[pos].cont=inc_inst();
	return NULL;
}

uint32_t buscar_victima_tlb(void){
	uint32_t min= TLB[0].cont;
	uint32_t victima=0;
	for(int i=1;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
		if(TLB[i].cont<min){
			victima = i;
			min=TLB[i].cont;
		}
	}
	log_info(logram,"la victima TLB es pid%i, pagina%i,frame %i",TLB[victima].pid,TLB[victima].pagina,TLB[victima].frame);
	return victima;	
}





int liberar(uint32_t direccion,t_pcb *PCB){
	if (direccion<tamHeap || direccion >=PCB->alocado -tamHeap || PCB->alocado ==0){return MATE_FREE_FAULT;}//direccion fuera del rango asignado
	int posicion = direccion-tamHeap;
	t_alloc heapInicial;
	t_alloc heapFinal;
	int posInicial=posicion;
	int posFinal=posicion;
	t_alloc heapAliberar;
	t_alloc heapAnterior;
	t_alloc heapSiguiente;
	t_alloc heapSubsiguiente;
	int cambionext =0;
	
	int cont=1;
	allocs(PCB);
	//busco el heap al que corresponde
	leer(&heapAliberar,tamHeap,posicion,PCB);
	heapInicial=heapAliberar;
	if (leer_memoria(&heapSiguiente,heapAliberar.nextAlloc,tamHeap,PCB)<0){return -1;}
	if (heapSiguiente.isFree==1 &&heapSiguiente.nextAlloc !=NULL){//esta libre y no es el ultimo, uso el que sigue
		if (leer_memoria(&heapSubsiguiente,heapSiguiente.nextAlloc,tamHeap,PCB)<0){return -1;}
		posFinal=heapSiguiente.nextAlloc;
		heapFinal=heapSubsiguiente;
		cambionext=1;
	}
	else {//es el heap que debo usar
		heapFinal=heapSiguiente;
		posFinal=heapAliberar.nextAlloc;
	}
	if (posicion > 0){
		if (leer_memoria(&heapAnterior,heapAliberar.prevAlloc,tamHeap,PCB)<0 || heapAliberar.prevAlloc>posicion){return -1;}
		if (heapAnterior.isFree==1){//el heap previo esta libre, es el primero
			heapInicial = heapAnterior;
			posInicial = heapAliberar.prevAlloc;
			cambionext=1;

		}
		else {
			heapInicial = heapAliberar;
		}
	}
	heapInicial.isFree=1;
	heapInicial.nextAlloc=posFinal;
	if (heapFinal.nextAlloc==NULL){//es el ultimo heap, debo liberar la memoria
		heapInicial.nextAlloc=0;
		if (posInicial > 0){
			liberarFrames(posInicial+tamHeap,PCB);
			escribir(&heapInicial,tamHeap, posInicial,PCB);
			allocs(PCB);
			return 0;
		}
		else{
			liberar_paginas(PCB);
			return 0;
		}

	}
	heapFinal.prevAlloc=posInicial;
	if (cambionext>0){
		escribir(&heapFinal,tamHeap,posFinal,PCB);
	}
	escribir(&heapInicial,tamHeap,posInicial,PCB);
	allocs(PCB);
/*	while (posicion+tamHeap < direccion){//la direccion no es el inicio del hueco, leo el siguiente heap
		posicion=heap.nextAlloc;
		leer(&heap,tamHeap,posicion,PCB);
		cont++;
	}
	if (posicion+tamHeap != direccion){return MATE_FREE_FAULT;} //la direccion no es el principio del hueco
	if (heap.isFree == 1){return 0;}//el hueco ya esta libre
	log_info(logger,"voy a liberar el heap %i en la pos %i",cont,posicion);
	unir_heap(heap,posicion,PCB);
*/
return 0;
}
void liberarFrames (int desde,t_pcb *PCB){
	if (metodo_asignacion == FIJA){
		PCB->alocado=desde;
		return;
	}
	if ((desde-1)/memoria_config->TAMANIO_PAGINA == PCB->alocado/memoria_config->TAMANIO_PAGINA){
		PCB->alocado=desde;
		return;
	}
	int inicio=(desde-1)/memoria_config->TAMANIO_PAGINA + 1;
	int hasta = PCB->paginas;

	//int cant= ((PCB->alocado+(PCB->alocado%memoria_config->TAMANIO_PAGINA))/memoria_config->TAMANIO_PAGINA) - (desde/memoria_config->TAMANIO_PAGINA);
	sem_wait(&sem_bitmap);
	for (inicio;inicio<hasta;inicio++){
		if(PCB->tabla_paginas[inicio].valido==1){
		mapaDeBits[PCB->tabla_paginas[inicio].frame]=0;
	}	
	}
	sem_post(&sem_bitmap);
	sem_wait(&sem_tablas);
	t_tabla_paginas *new = malloc (sizeof(t_tabla_paginas)*(hasta-inicio));
	for (int i=0;i<(hasta-inicio);i++){//copio los valores de la tabla existente
		new[i].frame = PCB->tabla_paginas[i].frame;
		new[i].indice=i;
		new[i].valido = PCB->tabla_paginas[i].valido;
		new[i].accedido = PCB->tabla_paginas[i].accedido;
		new[i].Modificado=PCB->tabla_paginas[i].Modificado;
		new[i].pcb=PCB;
		if (new[i].valido==1){
			frames[new[i].frame]=&new[i];
		}
	}
	free(PCB->tabla_paginas);
	PCB->tabla_paginas=new;
	PCB->alocado=desde;
	PCB->paginas=PCB->paginas-(hasta-inicio);
	sem_post(&sem_tablas);
	
	return ;
}

void unir_heap(t_alloc heap,uint32_t posicion, t_pcb *PCB){
	heap.isFree=1;
	t_alloc nextHeap;
	t_alloc prevHeap;
	t_alloc heapInicial=heap;
	t_alloc heapFinal = heap;
	uint32_t posInicial=posicion;
	uint32_t posFinal=posicion;

	if (posicion ==0){
		prevHeap.isFree=0;
	}
	else{
		leer(&prevHeap,tamHeap,heap.prevAlloc,PCB);
	}

	if (heap.nextAlloc == NULL){
		nextHeap.isFree =0;
	}
	else{
		leer(&nextHeap,tamHeap,heap.nextAlloc,PCB);
	}
	if (nextHeap.isFree==1){//el heap siguiente esta libre
		if (nextHeap.nextAlloc!=NULL){//hay un heap luego de este, y tengo que actualizar su valor de prevAlloc
			leer(&heapFinal,tamHeap,nextHeap.nextAlloc,PCB);
			posFinal=nextHeap.nextAlloc;
			heapFinal.prevAlloc=posicion;
			heapInicial.nextAlloc=posFinal;
		}
		else {
		heapFinal=nextHeap;
		posFinal=heap.nextAlloc;
		
		}
	}
	
	else {
		heapFinal=nextHeap;
		posFinal=heap.nextAlloc;
		
	}
	if (prevHeap.isFree==1){//el heap previo esta libre, es el inicial
		heapInicial = prevHeap;
		posInicial=heap.prevAlloc;
		
		
	}
	else{//el heap actual es el inicial
		
		
	}
	heapFinal.prevAlloc=posInicial;
	heapInicial.nextAlloc=posFinal;

	escribir(&heapInicial,tamHeap,posInicial,PCB);

	escribir(&heapFinal,tamHeap,posFinal,PCB);
	return;
}

void finalizar_proceso(t_pcb *pcb){
	quitar_de_lista(listaPCB,pcb);
	liberar_paginas(pcb);
	enviar_codigo(OP_FINALIZAR_PROCESO,socketSwamp);
	enviar_codigo(pcb->pid,socketSwamp);
	close(pcb->socket);
	free(pcb->frames);
	free(pcb);
}


void liberar_paginas(t_pcb *pcb){
	if(pcb->tabla_paginas==NULL){return;}
	//liberar swamp()
	for (int i=0;i<=pcb->alocado/memoria_config->TAMANIO_PAGINA;i++){
		if(pcb->tabla_paginas[i].valido==1){
			mapaDeBits[pcb->tabla_paginas[i].frame]=0;
		}
	}
	pcb->alocado=0;
	free(pcb->tabla_paginas);
	free (pcb->frames);
	pcb->tabla_paginas=NULL;
	pcb->frames=NULL;
	pcb->paginas=0;
}

void quitar_de_lista(t_list *lista, void *ptr){
    t_list *temp;
    for (int i=0;i<list_size(lista);i++){
        temp=list_get(lista,i);
        if (ptr==temp){
            list_remove(lista,i);
            return;
        }
    }
}

void generar_dump(void){
	FILE *fdump;
	char fname[500];
	char * path=memoria_config->PATH_DUMP_TLB;
	log_info(logger,"dumping por compactacion");
	char *tstamp = temporal_get_string_time("%d-%m-%y %H_%M_%S");
	sprintf(fname,"%s/Dump_especial_%s.txt",path,tstamp);
	fdump=fopen(fname,"w+");
	char text[100];
	sprintf(text,"Dump: %s\n",tstamp);
	fputs(text, fdump);
	sem_wait(&sem_tlb);
	for (int i=0;i<memoria_config->CANTIDAD_ENTRADAS_TLB;i++){
		if(TLB[i].pid==0){
			sprintf(text,"Entrada:%-4i Estado:Libre    Carpincho: -      Pagina: - Marco: -\n",i);
		}
		else{
			sprintf(text,"Entrada:%-4i Estado:Ocupado  Carpincho: %-4i     Pagina: %-4i Marco: %-4i\n",i,TLB[i].pid,TLB[i].pagina,TLB[i].frame);
		}
		fputs(text, fdump);
		
	}
	sem_post(&sem_tlb);
	free (tstamp);
	fclose(fdump);
	
}