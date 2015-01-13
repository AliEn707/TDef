/*
counting semaphore for threads
*/
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/sem.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

//#define t_sem
#include "t_sem.h"
//#include "engine.h"


t_sem_t t_semget(key_t key, int nsems, int semflg){
	t_sem_t sem;
	if ((sem=malloc(sizeof(struct t_sem_t)))==0)
		return 0;
	memset(sem,0,sizeof(struct t_sem_t));
	if (pthread_mutex_init(&sem->mutex, 0)!=0){
		free(sem);
		return 0;
		//add error show
	}
	if ((sem->val=malloc(sizeof(short)*nsems))==0)
		return 0;
	memset(sem->val,0,sizeof(short)*nsems);
//	t_sem_t $=semget(key, nsems, semflg); 
	return sem;
}

int t_semop(t_sem_t s, struct sembuf *sops, unsigned nsops){
//	int $=semop(semid, sops, nsops);
	if (sops->sem_op==0){
		int $_$=1;
		while($_$){
//			printf("set %d to %d have %d\n",sops->sem_num,sops->sem_op,s->val[sops->sem_num]);
			if (pthread_mutex_lock(&s->mutex)!=0)
				return -1;
			if (s->val[sops->sem_num]==0)
				$_$=0;
			if(pthread_mutex_unlock(&s->mutex)!=0)
				return -1;
			if ($_$)
				usleep(30000); //~30 tps
		}
		return 0;
	}
	if (sops->sem_op<0){
		int $_$=1;
		while($_$){
//			printf("set %d to %d have %d\n",sops->sem_num,sops->sem_op,s->val[sops->sem_num]);
			if (pthread_mutex_lock(&s->mutex)!=0)
				return -1;
			if (s->val[sops->sem_num]>=-sops->sem_op){
				s->val[sops->sem_num]+=sops->sem_op;
				$_$=0;
			}
			if(pthread_mutex_unlock(&s->mutex)!=0)
				return -1;
			if ($_$)
				usleep(30000); //~30 tps
		}
		return 0;
	}
	//otherwise we have sops->sem_op>0	
//	printf("set %d to %d have %d\n",sops->sem_num,sops->sem_op,s->val[sops->sem_num]);
	if (pthread_mutex_lock(&s->mutex)!=0)
		return -1;
	s->val[sops->sem_num]+=sops->sem_op;
	if(pthread_mutex_unlock(&s->mutex)!=0)
		return -1;
//	printf("work\n");
	return 0;
}

int t_semctl(t_sem_t s, int semnum, int cmd){
//	int $;//=semctl(semid, semnum, cmd);
	if (cmd==IPC_RMID){
		if(pthread_mutex_destroy(&s->mutex)!=0)
			return -1;
		free(s->val);
		free(s);
		return 0;
	}
	printf("not imlemented \n");
	return -1;
}
