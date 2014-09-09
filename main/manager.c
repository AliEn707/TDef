/*
manager
must:
parse manager.ini
get ftok by file "server"
	create semaphore set ftok 's'
	create shared memory ftok 'm' size = servnum*sizeof(char)
		set mem by 0
		set sem to 1
create socket
loop
	listen port
	on connect 
	check client auth (need to think how)
		get token (one int or longlong, for room data)
		fork()
			execv server -port -token, may be something else
			//write on server app
			//server after start get shmem and semaphore 
			//set shmem[port] to 1 and before exit to 0
			//when change use semaphore
	else
		drop connection

///////
messages and commands must be described in this file or another
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
//#include <netdb.h>

int recvData(int sock, void * buf, int size){
	int need=size;
	int get;
	get=recv(sock,buf,need,0);
	if (get<0)
		return -1;
	if (get==need)
		return get;
	printf("get not all\n");
	while(need>0){
		need-=get;
		if((get=recv(sock,buf+(size-need),need,0))<=0)
			return get;
	}
	return size;
}

int main() {
	FILE * manager_file;
	char buffer [100];
	manager_file = fopen ("manager.ini" , "r");
	int menport  = 7922, servnum  = 0, startport = 0;//default values
	if (manager_file == NULL) 
		perror ("Can't read config file manager.ini");
	else {
		while (!feof (manager_file)) {
			if (fgets (buffer , 100 , manager_file) == NULL ) //
				break;
			sscanf(buffer, "menport %d", &menport); //maybe 9738
			sscanf(buffer, "servnum  %d", &servnum);
			sscanf(buffer, "startport  %d", &startport);
		}
		fclose (manager_file);
	}
	
	key_t key_token = ftok("manager.ini", 100);//TODO: add correct filename ad path to file "server"
	struct sembuf sem_server;
	memset(&sem_server, 0, sizeof(sem_server));
	int sem_id = semget(key_token, 1, IPC_CREAT); //generate semaphore id
	sem_server.sem_num = 0; //index in sem_set
	sem_server.sem_op = 1; //add 1 to counter
	semop(sem_id, &sem_server, 1); //perform operation (sem_server.sem_op=1)
	int shared_id = shmget(key_token, servnum*sizeof(char), 0777|IPC_CREAT);//create shared memory
	char *ports_info = shmat(shared_id, 0, 0);
	if (ports_info == 0)
		perror("Failed to create shared memory");
	memset(ports_info, 0, servnum*sizeof(char));
		
	int listener;
	struct sockaddr_in addr;
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror("Failed to start socket");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(menport);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("Failed to bind");
	if (listen(listener, 1)<0)
		perror("Failed to listen");
	int sock = 0;
	while (1) {
		if ((sock = accept(listener, NULL, NULL))<0)
			perror("Failed to accept");
		char msg_type;//TODO: maybe fix!
		if (recvData(sock, &msg_type, sizeof(msg_type)) <= 0) {
			close(sock);
			continue;
		}
		//TODO: check client auth
		int room_data = 0;
		if (recvData(sock, &room_data, sizeof(room_data)) <= 0) {
			close(sock);
			continue;
		}
		sem_server.sem_op = -1;
		semop(sem_id, &sem_server, 1);
		int i = 0, flag = -1;
		for(; i < servnum; i++)
			if (ports_info[i] == 0) {
				flag = i;
				break; 
			}
		sem_server.sem_op = 1;
		semop(sem_id, &sem_server, 1);
		pid_t pid;
		if (flag != -1) {
			char port_arg[15], token_arg[15];
			switch (pid = fork()) { //create child process
				case -1:
					perror("Failed to fork");
					exit(1);//TODO: send error report
				case 0: //child process
					sprintf(port_arg, "%d", menport);
					sprintf(token_arg, "%d", room_data);
					execl("/bin/ls", "ls", 0, 0);//TODO
					//execl("server", "-port", port_arg, "-token", token_arg, 0);												
			}
		}
	}
	return 0;
}


