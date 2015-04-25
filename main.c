#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>


#define BUFFER_SIZE 20
#define BIG_PAUSE_COUNT 1400
#define SMALL_PAUSE_COUNT 380
#define LOOP_PAUSE 1400

typedef enum{inital, bigPause, smallPause, upDownVolume} tStateVolume;
typedef enum{initalPrg, loop} tStateProgram; 

tStateVolume volumeState=inital;
tStateProgram programState= initalPrg;

int buffer[BUFFER_SIZE];
int start=0, end=0;
pthread_mutex_t mutex, printMutex;
pthread_cond_t full_cv, empty_cv;


int bigPauseCount=0;
int smallPauseCount=0;
void volumeStateMachine(int i){
	//if(!i){volumeState=inital; return;}
	switch(volumeState){
		case inital:
			produceCommand(i);
			volumeState=bigPause;
			bigPauseCount=0;
			smallPauseCount=0;
			break;
		case bigPause:
			if(bigPauseCount==BIG_PAUSE_COUNT){
				produceCommand(i);
				volumeState=smallPause;
			}else{
				bigPauseCount++;
				//reamin in the same state//volumeState=bigPause;
			}
			break;
		case smallPause:
			if(smallPauseCount==SMALL_PAUSE_COUNT){
				//volumeState=upDownVolume;
				produceCommand(i);
				smallPauseCount=0;
			}else{
				smallPauseCount++;
				//reamin in same state
			}
			break;
		case upDownVolume:
			break;
	}

}

int loop_pause=0;
void programStateMachine(int i){
	switch(programState){
		case initalPrg:
			loop_pause=0;
			programState=loop;
			break;
		case loop:
			if(loop_pause==LOOP_PAUSE){
				produceCommand(i);
				loop_pause=0;
			}else{
				loop_pause++;
			}
			break;
	}

}
int consumeCommand(){
	int command;
	pthread_mutex_lock(&mutex);
	if(start==end){
		pthread_cond_wait(&empty_cv, &mutex);
	}
	command=buffer[end];
	end = ( end + 1 )% BUFFER_SIZE;
	pthread_cond_signal (&full_cv); 
	pthread_mutex_unlock(&mutex);
	return command;
}

void produceCommand(int command){
	pthread_mutex_lock(&mutex);
	if((start+1)%BUFFER_SIZE==end){
		pthread_mutex_lock(&printMutex);
		printf("-------------------------------------- !!!!! Big WARNING BUFFER IS FULL !!!!!--------------------------------------\n");
		pthread_mutex_unlock(&printMutex);
		pthread_cond_wait(&full_cv, &mutex);
	}
	buffer[start]=command;
	start=(start+1)%BUFFER_SIZE;
	pthread_cond_signal (&empty_cv); 
	pthread_mutex_unlock(&mutex);	
}

void producerThread(void* t){
	long id=(long)t;
	srand(time(NULL));
	int i=1;
	int command;
	while(1){
		command = rand()%10;
		//volumeStateMachine(command);
		programStateMachine(command);
		pthread_mutex_lock(&printMutex);
		//printf("[Producer]:	with id %d produced command	%d \n", id, command);
		pthread_mutex_unlock(&printMutex);
		i++;
		usleep(200);
		/*pthread_mutex_lock(&printMutex);
		printf("[Producer]");
		pthread_mutex_unlock(&printMutex);*/
	}

}


void consumerThread(void* t){
	long id=(long)t;
	int command, i=1;
	while(1){
		command = consumeCommand();
		pthread_mutex_lock(&printMutex);
		printf("[Consumer]:	with id %d consumed command	%d \n", id, command);
		pthread_mutex_unlock(&printMutex);
		i++;
		//usleep(1200);
		/*pthread_mutex_lock(&printMutex);
		printf("[Producer]");
		pthread_mutex_unlock(&printMutex);*/
	}
}


int main(){
	pthread_t consumer, producer;
	long consumerID, producerID;
	pthread_attr_t attr;
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&printMutex, NULL);
	pthread_cond_init (&empty_cv, NULL);
	pthread_cond_init (&full_cv, NULL);
	pthread_attr_init(&attr);
 	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
 	pthread_create(&consumer, &attr, consumerThread, (void *)consumerID);
 	pthread_create(&producer, &attr, producerThread, (void *)producerID);

 	pthread_join(consumer, NULL);
 	pthread_join(producer, NULL);



	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&printMutex);
	pthread_cond_destroy(&full_cv);
	pthread_cond_destroy(&empty_cv);
	pthread_exit(NULL);
	return 0;
		
}
