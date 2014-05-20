#include "grid.h"
#include "gridmath.h"
#include "file.h"

int main(){
	initGridMath();
//	gnode grid[100];
	gnode* grid;
//	gridsize=10;
//	memset(grid,0,sizeof(grid));
	loadConfig("test.cfg");
	grid=loadMap("test.mp");
	loadTypes("types.cfg");
	initArrays();
	spawnNpc(grid,3,0,1);
	int i;
/*	for(i=0;i<100;i++){
		grid[i].buildable=1;
		grid[i].id=i;
		if(rand()%100<10)
			grid[i].buildable=0;
	}
*/	int j;
	
	int a=aSearch(grid,grid+80,grid+3);
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
		printf("\n");
	}
	
	vec v={1.5,1.3};
	printf("%d\n",getGridId(v));
	printf("%d\n",a);
//	sleep(10);

	realizeMap(grid);
	realizeTypes();
	realizeArrays();
	return 0;
}	