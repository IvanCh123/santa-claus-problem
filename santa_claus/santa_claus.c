#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct{
	size_t duende;
	size_t reno;
  
	size_t cantidad_renos;
	size_t cantidad_duendes;
  
	sem_t santa_sem;
	sem_t reno_sem;
	sem_t duende_sem;
	pthread_mutex_t mutex;
}shared_data_t;  
 
typedef struct{
	size_t thread_id;
	shared_data_t* shared_data;
}private_data_t;
 
void prepare_sleigh(){
	printf("Santa: Preparando el trineo\n");

}

void help_elves(){
	printf("Santa: ayudando a los duendes\n");
}
    
void* empezar_reno(void* data)
{
	private_data_t* private_data = (private_data_t*)data;
	shared_data_t* shared_data = private_data->shared_data;
	
	while(true){
  		
		pthread_mutex_lock(&shared_data->mutex); //Abro region critica
    
		shared_data->reno+=1;
		if(shared_data->reno == 9){         //Pregunto si hay nueve renos 
			sem_post(&shared_data->santa_sem); //Mando señal a santa para decirle si hay nueve renos y despertarlo
		}
    
		pthread_mutex_unlock(&shared_data->mutex); //Cierro region
    
		printf("Reno %zu esperando por los demas\n",private_data->thread_id);
		sem_wait(&shared_data->reno_sem); //Pongo a esperar a los renos 
		
		sleep(20);
	}
}

void* empezar_duende(void* data)
{	
	private_data_t* private_data = (private_data_t*)data;
	shared_data_t* shared_data = private_data->shared_data;
	
	while(true){
		bool ayuda = random() % 100 < 10;
    
		if(ayuda){
			sem_wait(&shared_data->duende_sem);
			pthread_mutex_lock(&shared_data->mutex);
			shared_data->duende+=1;
  
			if(shared_data->duende == 3){
				sem_post(&shared_data->santa_sem);
			}else{
				sem_post(&shared_data->duende_sem);
			}
      
			pthread_mutex_unlock(&shared_data->mutex);
  	
			printf("Duende %zu pide ayuda a santa\n", private_data->thread_id);
			sleep(10);
      
			pthread_mutex_lock(&shared_data->mutex); 
			shared_data->duende--;
			if(shared_data->duende == 0){
				sem_post(&shared_data->duende_sem);
			}
			pthread_mutex_unlock(&shared_data->mutex);
			
		}
		printf("Duende %zu trabajando\n",private_data->thread_id);
		sleep(2 + random() % 5);		
	}
}
void* empezar_santa(void* data)
{
	shared_data_t* shared_data = (shared_data_t*)data;
	
	while(true){
		sem_wait(&shared_data->santa_sem);  // Pongo a santa a esperar 
		pthread_mutex_lock(&shared_data->mutex);  // Abro region critica 
		
		if (shared_data->reno >= 9){  //Si la cantidad de renos es mayor o igual a nueve, preparo el trineo
			prepare_sleigh();
      
			for(size_t i=0;i<shared_data->cantidad_renos;i++){ 
				sem_post(&shared_data->reno_sem);	 //Espero a que lleguen los nueve renos
			}
			printf("Se repartieron regalos por el mundo\n");
			shared_data->reno -= 9;
		
		}else if(shared_data->duende == 3){ //Si los duendes son 3 
			help_elves(); // Ayudo a los duendes
		}

		pthread_mutex_unlock(&shared_data->mutex);// Se termina region critica
	}
}

int main(void)
{
	//Definicion de variables
	shared_data_t shared_data;
  
	shared_data.cantidad_renos = 9;
	shared_data.cantidad_duendes = 10;
	shared_data.duende = 0;
	shared_data.reno = 0;
  
	//Se inicializan los semaforos a utilizar
	pthread_mutex_init(&shared_data.mutex, NULL);
	sem_init(&shared_data.santa_sem,0,0);
	sem_init(&shared_data.reno_sem,0,0);
	sem_init(&shared_data.duende_sem,0,1);
  
	//Se reserva memoria para los renos y duendes
	pthread_t* renos = (pthread_t*) malloc(shared_data.cantidad_renos * sizeof(pthread_t));
	pthread_t* duendes = (pthread_t*) malloc(shared_data.cantidad_duendes * sizeof(pthread_t));
	pthread_t* santa = (pthread_t*) malloc(sizeof(pthread_t));
  
	
	pthread_create(santa, NULL,empezar_santa,&shared_data);
  
	private_data_t* private_renos = (private_data_t*) malloc(shared_data.cantidad_renos * sizeof(private_data_t));
	//Crea los renos
	for(size_t i=0;i<shared_data.cantidad_renos;i++){
		private_renos[i].thread_id = i;
		private_renos[i].shared_data = &shared_data;
		pthread_create(&renos[i], NULL, empezar_reno, &private_renos[i]);
	}	
	
	private_data_t* private_duendes = (private_data_t*) malloc(shared_data.cantidad_duendes * sizeof(private_data_t));
	//Crea los duendes
	for(size_t j=0;j<shared_data.cantidad_duendes;j++){
		private_duendes[j].thread_id = j;
		private_duendes[j].shared_data = &shared_data;
		pthread_create(&duendes[j], NULL, empezar_duende, &private_duendes[j]);
	}
	
  
	//Espera todos los renos
	for(size_t i=0;i<shared_data.cantidad_renos;i++){
		pthread_join(renos[i], NULL);
	}	
  
	//Espera los duendes
	for(size_t i=0;i<shared_data.cantidad_duendes;i++){
		pthread_join(duendes[i], NULL);
	}
	//Libero memoria
	free(renos);
	free(duendes);
	free(santa);
	free(private_duendes);
	free(private_renos);

	return 0;
}
