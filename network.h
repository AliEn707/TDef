
int recvData(int sock, void * buf, int size);

int processMessage(worker_arg * data,char type);

int startServer(int port);

int realizeServer();

int tickSendNpc(gnode* grid,npc* n);

int tickSendTower(gnode* grid,tower* t);

int tickSendBullet(gnode* grid,bullet * b);