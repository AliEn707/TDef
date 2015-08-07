#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include <netdb.h>

#include "manager.h"

#define SLEEP_TIME (60*5)
#define TMP_FILE "/tmp/update.tmp"

#define PUBLIC_HOST "localhost"
#define PUBLIC_PORT 7000



#define MESSAGE_UPDATE 3

#define MESSAGE_UPDATE_NPC_TYPES 1
#define MESSAGE_UPDATE_TOWER_TYPES 2
#define MESSAGE_UPDATE_BULLET_TYPES 3
#define MESSAGE_UPDATE_MAPS 4



static inline int connectToHost(char* host, int port){
	int sockfd;
	struct sockaddr_in servaddr;
	struct hostent *server;
	server = gethostbyname(host);
	if (server == NULL) {
		perror("gethostbyname");
		return -1;
	}
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return -1;
	}
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	memcpy((char *)&servaddr.sin_addr.s_addr,(char *)server->h_addr, server->h_length);
//	servaddr.sin_addr.s_addr=inet_addr("172.16.1.40");//argv[1]);
	servaddr.sin_port=htons(port);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))<0){
		perror("connect");
		return -2;
	}	
	return sockfd;

}

static inline void copyFile(char *dest, char*src){
	FILE* f, *f2=fopen(dest,"wt+");
	int size;
	if (f2==0)
		return;
	if ((f=fopen(src,"rt"))==0){
		fclose(f2);
		return;
	}
	do{
		char buf[100];
		size=fread(buf,1,sizeof(buf),f);
		fwrite(buf,1,size,f2);
	}while(size>0);
	fclose(f);
	fclose(f2);
}

static inline int fileTime(char* path){
	struct stat st;
	if (stat(path,&st)<0)
		return 0;
	return st.st_mtime;
}

static inline void updateTypes(int sock,char msg_type, char* path){
	int timestamp=fileTime(path);
	int size;
	sendData(sock, &msg_type, sizeof(msg_type));
	sendData(sock, &timestamp, sizeof(timestamp));
	if(recvData(sock,&size,sizeof(size))<=0)
		return;
	if (size==0)
		return;
	FILE* f=fopen(TMP_FILE,"wt+");
	if (f==0)
		return;
	while(size>0){
		char buf[10];
		do{
			int get=size>sizeof(buf)?sizeof(buf):size;
			recvData(sock,buf,get);
			fwrite(buf,1,get,f);
			size-=get;
		}while(size>0);
		recvData(sock,&size,sizeof(size));
	}
	fclose(f);
	copyFile(path,TMP_FILE);
}

static inline void updateMaps(int sock){
	char msg_type=MESSAGE_UPDATE_MAPS;
	int size=1;
	char path[150];
	FILE* f;
	sendData(sock, &msg_type, sizeof(msg_type));
	if(recvData(sock,&size,sizeof(size))<=0)
		return;
	while(size>0){//lets get map
		char buf[100];
		//try to get name
		memset(buf,0,sizeof(buf));
		if(recvData(sock,buf,size)<=0)
			return;
		sprintf(path,"../maps/%s.mp",buf);
		int timestamp=fileTime(path);
		sendData(sock, &timestamp, sizeof(timestamp));
		//try to get size of file
		if(recvData(sock,&size,sizeof(size))<=0)
			return;
		if (size==0)
			return;
		if ((f=fopen(TMP_FILE,"wt+"))==0)
			return;
		while(size>0){//het parts of file
			while(size>0){//get 1 part of map
				int get=size>sizeof(buf)?sizeof(buf):size;
				if (recvData(sock,buf,get)<=0)
					break;
				fwrite(buf,1,get,f);
				size-=get;
			}
			if(recvData(sock,&size,sizeof(size))<=0)
				break;
		}	
		fclose(f);
		copyFile(path,TMP_FILE);
		if(recvData(sock,&size,sizeof(size))<=0)
				return;
	}
}


static void * updater(void * arg) {
	char msg_type=MESSAGE_UPDATE;
	while(stop==0){
		updating=1;
		
		while (canUpdate() || 1){
			sleep(1);
		}
		int sock=connectToHost(PUBLIC_HOST,PUBLIC_PORT);
		sendData(sock, &msg_type, sizeof(msg_type));
		updateTypes(sock, MESSAGE_UPDATE_NPC_TYPES, "../data/types/npc.cfg");
		updateTypes(sock, MESSAGE_UPDATE_TOWER_TYPES, "../data/types/tower.cfg");
//		updateTypes(sock, MESSAGE_UPDATE_BULLET_TYPES, "../data/types/bullet.cfg");
		updateMaps(sock);
		close(sock);
		
		updating=0;
		
		sleep(SLEEP_TIME);
	}
	return 0;
}


pthread_t startUpdater(int* arg){
	pthread_t th = 0;
	if (pthread_create(&th, 0, updater, arg) != 0)
		return 0;
	return th;
}