//recv size data to buf
int recvData(int sock, void * buf, int size);
int _sendData(int sock, void * buf, int size);

int processMessage(worker_arg * data,char type);

int startServer(int port,gnode* grid);

int realizeServer();

int tickSendNpc(gnode* grid,npc* n);

int tickSendTower(gnode* grid,tower* t);

int tickSendBullet(gnode* grid,bullet * b);

int sendPlayers(int sock,int id);

int sendTest(int sock);

int networkAuth(worker_arg *data);

//send time before game start
int networkWaitingTime(worker_arg *data);

int connectToHost(char* host, int port);

//function for sending take and free to manager about mapserver port
int networkPortTake();
int networkPortFree();