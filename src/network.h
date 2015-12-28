#define PACKET_SIZE 250
typedef
struct{
	int sock;
	int size;
	char buf[PACKET_SIZE];
} packet;

//recv size data to buf
int recvData(int sock, void * buf, int size);
int _sendData(int sock, void * buf, int size);

//create new packet
packet* packetNew(int sock);
//add data to packet, if size of packet more than PACKET_SIZE, send it and start new ( )
int packetAdd(packet *p, void* data, int size);
//send packet and free memory ( <=0 for error)
int packetFinish(packet *p);

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


int wrongByteOrder();

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define biteSwap(a) ({ typeof(a) out=a;\
	if (sizeof(a)==2){\
		short val=*((short*)&(a));\
		val=(val << 8) | ((val >> 8) & 0xFF);\
		out=*((typeof(a)*)&val);\
	}else if (sizeof(a)==4){\
		int val=*((int*)&(a));\
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );\
		val = (val << 16) | ((val >> 16) & 0xFFFF);\
		out=*((typeof(a)*)&val);\
	} else if (sizeof(a) == 8){\
		long long val=*((long long*)&(a));\
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );\
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );\
		val = (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);\
		out=*((typeof(a)*)&val);\
	}\
out;})
//TODO: change to faster version

#else

#define biteSwap(a) (a)

#endif
