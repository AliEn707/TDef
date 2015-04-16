
typedef 
struct bintree{
	void * data;
	struct bintree * next[2];
} bintree;



int bintreeAdd(bintree* root,int key,void* data);

void * bintreeGet(bintree* root, int key);

int bintreeDel(bintree* root, int key, void (f)(void*v));

void bintreeErase(bintree * root,void (f)(void*v));
