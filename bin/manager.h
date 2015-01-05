
#include <pthread.h>


int recvData(int sock, void * buf, int size);

void * manager(void *);
void DestroyWorkThread();
pthread_t InitWorkThread();



#define sendData(sock,buf,need) send(sock,buf,need,MSG_NOSIGNAL)

