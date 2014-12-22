#include "grid.h"
#include "file.h"
#include "gridmath.h"
#include "threads.h"
#include "network.h"
#include "engine.h"
#include "engine_tower.h"
#include "engine_npc.h"
#include "engine_bullet.h"

#define PUBLIC_HOSTNAME "localhost"//"192.168.56.56"//
#define PUBLIC_PORT 7000


#define MESSAGE_ROOM_STATUS 1
#define MESSAGE_ROOM_RESULT 2


//room statuses
#define ROOM_PREPARE 1 //wait for proceed
#define ROOM_RUN 2	//allready played
#define ROOM_FAIL 0	//cant create room
#define ROOM_ERROR 3	//need to try create


#define sendData(sock,x,y) if(send(sock,x,y,MSG_NOSIGNAL)<=0) return -1 
static int publicConnect(){
	int sockfd;
	struct sockaddr_in servaddr;
	struct hostent *server;
	server = gethostbyname(PUBLIC_HOSTNAME);
	if (server == NULL) {
		perror("gethostbyname");
		return -1;
	}
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return -1;
	}
	//set sock to config struct
	config.game.sock=sockfd;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	memcpy((char *)&servaddr.sin_addr.s_addr,(char *)server->h_addr, server->h_length);
//	servaddr.sin_addr.s_addr=inet_addr("172.16.1.40");//argv[1]);
	servaddr.sin_port=htons(PUBLIC_PORT);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))<0){
		perror("connect");
		return -2;
	}	
	return sockfd;
}


int publicGetGame(){
	int sockfd;
	char mes;
	short status;
	sockfd=publicConnect();
	if (sockfd<=0)
		return -1;
	printf("connected\n");
	mes=MESSAGE_ROOM_STATUS;
	sendData(sockfd,&mes,sizeof(mes));
	printf("send mes %d\n",mes);
	sendData(sockfd,&config.game.token,sizeof(config.game.token));
	printf("token mes %d\n",config.game.token);
	sendData(sockfd,&config.game.port,sizeof(config.game.port));
	printf("port mes %d\n",config.game.port);
	status=ROOM_RUN;
	sendData(sockfd,&status,sizeof(status));
	printf("status mes %d\n",status);
	printf("sended\n");
	short l_l;
	if (recvData(sockfd,&l_l,sizeof(l_l))<0) 
		return -1;
	if (recvData(sockfd,config.game.map,l_l)<0)
		return -1;
	printf("map name %s\n",config.game.map);
	
	//n=recv(sockfd,&mes,sizeof(mes),0);
//	if (n<0)
//		return -1;
//	else
//		if (n>0)
//			return n;
	
	return 0;
}

int publicSendResults(){
	int sockfd=config.game.sock;
	char mes=MESSAGE_ROOM_RESULT;
	short status=ROOM_FAIL;
	if (send(sockfd,&mes,sizeof(mes),MSG_NOSIGNAL)<=0){
		sockfd=publicConnect();
		if (sockfd<=0)
			return -1;
		sendData(sockfd,&mes,sizeof(mes));
		sockfd=config.game.sock;
	}
	sendData(sockfd,&config.game.token,sizeof(config.game.token));
	//add some data
	
	//at the end send status
	sendData(sockfd,&status,sizeof(status));
	close(sockfd);
	return 0;
}

int publicSendThreadResult(){
	
	return 0;
}
