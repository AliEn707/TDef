#include <GL/glut.h>

#include "../grid.h"
#include "../gridmath.h"
#include "../engine.h"
#include "../engine_npc.h"
#include "../engine_tower.h"
#include "../engine_bullet.h"
#include "../file.h"

int window_w=512, window_h=512;	// Initial Size of the Window	

void drawSquare(float y,float x){
	glBegin(GL_LINES);
		glVertex2f(y-0.5,(x-0.5));
		glVertex2f(y-0.5,(x+0.5));
		glVertex2f(y-0.5,(x+0.5));
		glVertex2f(y+0.5,(x+0.5));
		glVertex2f(y+0.5,(x+0.5));
		glVertex2f(y+0.5,(x-0.5));
		glVertex2f(y+0.5,(x-0.5));
		glVertex2f(y-0.5,(x-0.5));
		
	glEnd();
	//glTranslatef(y,-x,0);
	//glutWireIcosahedron();
}

void pinfo(){
	int i=0,
		j=0,
		k=0;
	printf("Towers\t\t\tNpcs\t\t\tBullets\n");
	while(i<config.tower_max||
		j<config.npc_max||
		k<config.bullet_max){
		for(;config.tower_array[i].id<=0 && i<config.tower_max;i++);
		if (i<config.tower_max){
			printf("%d(%d)%d ",config.tower_array[i].id,
					config.tower_array[i].position,
					config.tower_array[i].type!=BASE?
						config.tower_array[i].health:
						config.players[config.tower_array[i].owner].base_health
					);
			i++;
		}
		printf("|\t\t\t");
		for(;config.npc_array[j].id<=0 && j<config.npc_max;j++);
		if (j<config.npc_max){
			printf("%d(%g,%g)%d %d %p %p",config.npc_array[j].id,
					config.npc_array[j].position.x,
					config.npc_array[j].position.y,
					config.npc_array[j].health,
					config.npc_array[j].status,
					config.npc_array[j].ttarget,
					config.npc_array[j].ntarget
					);
			j++;
		}
		printf("|\t\t\t");
		for(;config.bullet_array[k].id<=0 && k<config.bullet_max;k++);
		if (k<config.bullet_max){
			printf("%d(%g,%g) ",config.bullet_array[k].id,
					config.bullet_array[k].position.x,
					config.bullet_array[k].position.y
					);
			k++;
		}
		printf("\n");
	}
	
		
}


void drawGrid(gnode* grid){
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
//			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
			printf("%c ",
					grid[to2d(i,j)].tower!=0?
						grid[to2d(i,j)].tower->type==BASE?
							'B':
						'T':
						grid[to2d(i,j)].npcs[0]==0?
							grid[to2d(i,j)].walkable<1?
								'X':
							'O':
						'N');
		printf("\n");
	}		
}

void drawGridGl(gnode* grid){
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			grid[to2d(i,j)].tower!=0?
						grid[to2d(i,j)].tower->type==BASE?
							glColor3f(1,1,0):
						glColor3f(0,1,0):
						grid[to2d(i,j)].npcs[0]==0?
							grid[to2d(i,j)].walkable<1?
								glColor3f(1,0,0):
							glColor3f(1,1,1):
						glColor3f(1,1,1);
//			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
			drawSquare(getGridx(to2d(i,j)),getGridy(to2d(i,j)));
		}
		//printf("\n");
	}		
}



void tickDraw(gnode*grid,npc* n){
	glBegin(GL_POINTS);
	//car* Car;
	//for(Car=getCar(cars->next);Car!=cars;Car=getCar(Car->next))
	
		glVertex2f(n->position.x,n->position.y);
		glVertex2f(0,0);
		
	
	glEnd();
	}


void render(gnode * grid)
{
	int j;
	glMatrixMode(GL_MODELVIEW);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	
	glBegin(GL_POINTS);
		glVertex2f(5,-1.4);
	glEnd();
	
	glTranslatef(config.gridsize/2,config.gridsize/2,0);
	//glRotatef(60,1,0,0);
	glScalef(1,0.5,1);
	glRotatef(-45,0,0,1);
	glTranslatef(-config.gridsize/2,-config.gridsize/2,0);
	
	

	//glColor3f(rand()%100/100.0,rand()%100/100.0,rand()%100/100.0);
	glColor3f(1,1,1);
	forEachNpc(0,tickDraw);
	drawGridGl(grid);
	printf("\n");
	glutSwapBuffers();
}


void idle(gnode * grid){
	
}


void keyboard(unsigned char c,int x,int y)
{
	if (c) printf("21\n");
 
	
}
//////////////////////////////////////////
void init()
{
	// create a world with dimensions x:[-SIM_W,SIM_W] and y:[0,SIM_W*2]
	
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluOrtho2D(-config.gridsize/2.0,config.gridsize/2.0,-config.gridsize/2.0,config.gridsize/2.0);
	glOrtho(0,config.gridsize,0,config.gridsize,-10,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPointSize(5.f);
	
	
//	glutMotionFunc(motion);
	//glutMouseFunc(mouse);

	
}

void clearAll(gnode * grid){
	realizeMap(grid);
	realizeTypes();
	realizeArrays();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(window_w, window_h);
	glutCreateWindow("SPH");

//	glutDisplayFunc(render);
//	glutKeyboardFunc(keyboard);
//	glutIdleFunc(idle);
	//glutMotionFunc(motion);
	//glutMouseFunc(mouse);
	//////////////////////
	gnode* grid;
	initGridMath();
//	loadConfig("../test.cfg");
	loadTypes("../types.cfg");
	grid=loadMap("../test.mp");
	//config.player_max=4;
//	timePassed(0);
	npc* n=spawnNpc(grid,4,1,1);
	npc* n2=spawnNpc(grid,5,1,2);
	spawnNpc(grid,6,0,3);
	setupPlayer(1,1,2000,0);
	setupPlayer(2,0,1800,0);
	spawnTower(grid,75,1,BASE);
	spawnTower(grid,22,1,2);
	
	npc* n3=spawnNpc(grid,42,0,2);
	////////////////////
//	atexit(clearAll);
	init();
//	sysInit();
	//glutMainLoop();	
	
	while(1){
		timePassed(0);
		drawGrid(grid);
		
		processWaves(grid);
		
		forEachNpc(grid,tickDiedCheckNpc);
		forEachTower(grid,tickDiedCheckTower);
			
		forEachNpc(grid,tickCleanNpc);
		forEachTower(grid,tickCleanTower);
		forEachBullet(grid,tickCleanBullet);
			
		forEachNpc(grid,tickMoveNpc);
		forEachNpc(grid,tickTargetNpc);
		forEachNpc(grid,tickAttackNpc);
		forEachTower(grid,tickAttackTower);
		forEachBullet(grid,tickProcessBullet);
		
		forEachNpc(grid,tickMiscNpc);
		forEachTower(grid,tickMiscTower);
		forEachBullet(grid,tickMiscBullet);
		
		int z;
		//z=timePassed(1);
		printf("time %d",z);
		
		
		pinfo();
		render(grid);
		syncTPS();
		//usleep(100000);
	}
	
	
	clearAll(grid);
	
	return 0;
}
