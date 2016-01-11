
#include <pthread.h>
#include "../src/biteswap.h"

int recvData(int sock, void * buf, int size);

void * manager(void *);
void DestroyWorkThread();
pthread_t InitWorkThread();
int canUpdate();
int checkUpdate();
void setUpdate(short a);

int stop;

#define sendData(sock,x) ({typeof(x) _x=biteSwap(x);send(sock,&_x,sizeof(_x),MSG_NOSIGNAL);})

