#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "perf_test.h"
#include "manager.h"





static int sock;
static int client;
	
//time passed after previous call of function
static int timePassed(struct timeval * t){
	//config.time  struct timeval
	struct timeval end;
	gettimeofday(&end, NULL);
	int out=((end.tv_sec - t->tv_sec)*1000000+
			end.tv_usec - t->tv_usec);
	memcpy(t,&end,sizeof(end));
	return out;
}

static void syncTPS(int z,int TPS){
	if((z=(1000000/TPS)-z)>0){
		usleep(z);
	}
}

char map[50]="4";
#define MESSAGE_ROOM_STATUS 1

static int proceedServerMessage(char msg_type){
//	char msg;
	short l_l,status;
	int token;
	int port;
	if (msg_type==MESSAGE_ROOM_STATUS){ //packet [mes(char)token(int)status(short)]
		recvData(sock,&token,sizeof(token));
		recvData(sock,&port,sizeof(port)); //int
		recvData(sock,&status,sizeof(status)); //short
		
		l_l=strlen(map);
		sendData(sock,&l_l,sizeof(l_l));
		sendData(sock,map,l_l);
		//send data to client
		//----------------
		if(sendData(client,&port,sizeof(port))<=0);
//			DestroyWorkThread();
		return 0;
	}
	return -1;
	/*
	if (msg_type==MESSAGE_ROOM_RESULT){ //packet [mes(char)token(int)playertoken(int) ..
		printf("get room result\n");
		recvData(w->sock,&token,sizeof(token));
		semop(config.serverworker.room_sem,&sem[0],1);
		r_r=roomGetByToken(token);
		semop(config.serverworker.room_sem,&sem[1],1);
		//add another
		if (r_r==0){
			printf("cant find room\n");
			return 1;
		}
		//add some data
		
		//at the end get status
		recvData(w->sock,&r_r->status,sizeof(r_r->status)); //short
//		printf("get n set r status %d\n",r_r->status);
		return 0;
	}
//	printf("end of mes recv\n");
	return 0;
	*/
}

void * perfTest(void * arg){
	char** argv=arg;
	int listener,l;
	struct sockaddr_in addr;
	int menport=11111;
	int publicport=7000;
	int TPS=4;
	struct timeval tv={0,0};
	timePassed(&tv);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(menport);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if ((l = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("Failed to start socket");
	if (bind(l, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("Failed to bind");
	if (listen(l, 1)<0)
		perror("Failed to listen");
	if ((client = accept(l, NULL, NULL))<0)
				perror("Failed to accept");
	printf("client connected\n");
	short l_l;
	recvData(client,&l_l,sizeof(l_l));
	recvData(client,map,l_l);
	
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("Failed to start socket");
	addr.sin_port = htons(publicport);
	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("Failed to bind");
	if (listen(listener, 1)<0)
		perror("Failed to listen");
	fd_set read_fds;
//	struct timeval tv;
//	tv.tv_sec = 0;
//	tv.tv_usec = 25000;
	char msg_type;
	int stop=0;
	while(stop==0){
		FD_ZERO(&read_fds);
		FD_SET(listener, &read_fds);
		if (select (listener + 1, &read_fds, 0, 0, 0/*&tv*/) > 0) {
			if ((sock = accept(listener, NULL, NULL))<0)
				perror("Failed to accept");
			//get message from server
			if (recv(sock,&msg_type,sizeof(msg_type),0)>0){
				printf("got %d\n",msg_type);
				//get message need to proceed
				proceedServerMessage(msg_type);
			}
			close(sock);
		}
		
		syncTPS(timePassed(&tv),TPS);
	}
	close(l);
	close(listener);
	
	return 0;
}

pthread_t InitPerfTest(void * arg){
	pthread_t th;
		if (pthread_create(&th, 0, perfTest, arg) != 0)
			return 0;
	return th;
}

