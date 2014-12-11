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
#include <sys/wait.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define MANAGER "manager.ini"

#define sendData(sock,buf,need) send(sock,buf,need,MSG_NOSIGNAL)

int stop = 0;

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


void * manager(void * arg) {
	FILE * manager_file;
	char buffer [100];
	struct sembuf sem_server={0,0,0};
	int menport  = 7922, servnum  = 0, startport = 0;//default values
	int TPS=10;  //ticks per sec
	struct timeval tv={0,0};
	timePassed(&tv);
	manager_file = fopen (MANAGER, "r");
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
	
	key_t key_token = ftok(MANAGER, 100);//TODO: add correct filename ad path to file "server"
	memset(&sem_server, 0, sizeof(sem_server));
	int sem_id = semget(key_token, 1, 0755|IPC_CREAT); //generate semaphore id
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
	if (fcntl(listener, F_SETFD, fcntl(listener, F_GETFD) | FD_CLOEXEC) == -1)
				perror("Failed to set socket attributes");
	addr.sin_family = AF_INET;
	addr.sin_port = htons(menport);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("Failed to bind");
	if (listen(listener, 1)<0)
		perror("Failed to listen");
	int sock = 0;
	fd_set read_fds;
//	struct timeval tv;
//	tv.tv_sec = 0;
//	tv.tv_usec = 25000;
	while (!stop) {
		FD_ZERO(&read_fds);
		FD_SET(listener, &read_fds);
		if (select (listener + 1, &read_fds, 0, 0, 0/*&tv*/) > 0) {
			if ((sock = accept(listener, NULL, NULL))<0)
				perror("Failed to accept");
			char msg_type;//TODO: maybe fix!
			//TODO: client host == public check
			if (fcntl(sock, F_SETFD, fcntl(sock, F_GETFD) | FD_CLOEXEC) == -1)
				perror("Failed to set socket attributes");
			if (recvData(sock, &msg_type, sizeof(msg_type)) <= 0) {
				close(sock);
				continue;
			}
			if (msg_type==99){// ascii 'c' 
	//			printf("get msg %d\n",msg_type);
				//TODO: check client auth
//				if (err){
//					msg_type='e';
//					sendData(sock, &msg_type, sizeof(msg_type));
//					close(sock);
//				}
				int room_data = 0;
				if (recvData(sock, &room_data, sizeof(room_data)) <= 0) {
					close(sock);
					continue;
				}
	//			printf("get token %d\n",room_data);
				sem_server.sem_op = -1;
	//			printf("sem %d\n",semctl(sem_id,0,GETVAL));
	//			perror("sem");
				semop(sem_id, &sem_server, 1);
				int i = 0, flag = -1;
				for(; i < servnum; i++)
					if (ports_info[i] == 0) {
						ports_info[i] = -1;
						flag = i+startport;
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
							sprintf(port_arg, "%d", flag);
							sprintf(token_arg, "%d", room_data);
							printf("!!!  port  %d token %d\n",flag,room_data);
							if (execlp("./server", "./server", "-port", port_arg, "-token", token_arg, NULL) < 0) {//if (execlp("/bin/ls", "ls", 0, 0) < 0) {
								msg_type='e';
								sendData(sock, &msg_type, sizeof(msg_type));
								close(sock);
							}
							break;
						default:
							close(sock);											
					}
				}
			}
			if (msg_type==115){// ascii 's' 
				sem_server.sem_op = -1;
				semop(sem_id, &sem_server, 1);
				int i = 0, flag = -1;
				for(; i < servnum; i++)
					if (ports_info[i] != 0) {
						flag++;
					}
				sem_server.sem_op = 1;
				semop(sem_id, &sem_server, 1);
				//send num of free servers
				sendData(sock, &flag, sizeof(flag));//int
				close(sock);
			}
			if (msg_type==105){// ascii 'i' 
				sendData(sock, &menport, sizeof(menport));//int
				sendData(sock, &servnum, sizeof(servnum));//int
				sendData(sock, &startport, sizeof(startport));//int
				close(sock);
			}
			if (msg_type==114){// ascii 'r' 
			}
			if (msg_type==117){// ascii 'u' 
			}
		}
		syncTPS(timePassed(&tv),TPS);
		waitpid(0, 0, WNOHANG);
	}
	waitpid(0, 0, WNOHANG);
	close(listener);
	if (semctl(sem_id, 0, IPC_RMID) == -1)
		perror("semctl in manager");
	if (shmdt(ports_info) == -1)
		perror("shmdt in manager");
	if (shmctl(shared_id, IPC_RMID, 0) == -1)
		perror("shmctl in manager");
	printf("Manager exited\n");
	return 0;
}

// функция для остановки потоков и освобождения ресурсов
void DestroyWorkThread()
{
	stop = 1;
}
 
// функция которая инициализирует рабочие потоки
pthread_t InitWorkThread()
{
	pthread_t th = 0;
	if (pthread_create(&th, 0, manager, 0) != 0)
		return 0;
	return th;
}

