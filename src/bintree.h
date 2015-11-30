
typedef 
struct bintree{
	void * data;
	struct bintree * next[2];
} bintree;

typedef long long bintree_key;

int bintreeAdd(bintree* root,bintree_key key,void* data);

void * bintreeGet(bintree* root, bintree_key key);

int bintreeDel(bintree* root, bintree_key key, void (f)(void*v));

void bintreeErase(bintree * root,void (f)(void*v));
