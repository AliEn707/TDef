#include <sys/utsname.h>

char* systemArch(){
	static struct utsname buf;
	uname(&buf); 
	return buf.machine;
}