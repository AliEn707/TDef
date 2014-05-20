#include "grid.h"
#include "gridmath.h"
#include "file.h"

int main(){
	initGridMath();
//	gnode grid[100];
	gnode* grid;
//	gridsize=10;
//	memset(grid,0,sizeof(grid));
	
	grid=loadMap("test.mp");
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
	
	realizeMap(grid);
	printf("%d\n",a);
}