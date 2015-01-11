#ifndef t_sem
struct sembuf {
	short sem_num;
	short sem_op;
	short flags;
};
#endif

struct t_sem_t {
	pthread_mutex_t mutex;
	short *val;
};

//typedef int t_sem_t;
typedef struct t_sem_t* t_sem_t;
//#define t_sem_t int
/*
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
*/
struct t_sem_struct{
	t_sem_t send;
	t_sem_t player;
} t_sem;

#define IPC_RMID 4096
#define IPC_CREAT  0x0200
#define IPC_PRIVATE 0

#define t_semop semop
#define t_semctl semctl
#define t_semget semget



#undef t_semop
#undef t_semctl
#undef t_semget

int t_semop(t_sem_t semid, struct sembuf *sops, unsigned nsops);

t_sem_t t_semget(key_t key, int nsems, int semflg);

int t_semctl(t_sem_t semid, int semnum, int cmd);
