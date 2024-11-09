# include "iGraphics.h"
#include <stdlib.h>
#include "structures.cpp"


gameObject spaceship;
object_properties player;
const int total_particle = 1000;
particle flare[total_particle];
const double dt = 0.005;
const double acceleration = 20000;
const double max_velocity = 1000;
const double friction = 1000;
const double particle_life = 0.05;



void inititalize_gameObjects(gameObject *object, char filename[]);
void Draw_gameObject(object_properties player);
void Draw_flare();
void thrust();
void update();
void start();



void iDraw() {
	iClear();
	Draw_flare();
	Draw_gameObject(player);
}



/*
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouseMove(int mx, int my) {
	player.angle = atan2(my-player.position.y,mx-player.position.x);
}

/*
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		//place your codes here
		//	printf("x = %d, y= %d\n",mx,my);

	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		//place your codes here

	}
}

/*
	function iKeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed.
*/
void iKeyboard(unsigned char key) {
	if (key == 'q') {
		exit(0);
	}

	if(key == ' ')thrust();
}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
	*/
void iSpecialKeyboard(unsigned char key) {

	if (key == GLUT_KEY_END) {
		exit(0);
	}
	//place your codes for other keys here
}


int main() {
	
	inititalize_gameObjects(&spaceship,"assets/spaceship.txt");
	start();
	iSetTimer(dt*1000,update);
	iInitialize(1080, 720, "demo");
	

	return 0;
}


void inititalize_gameObjects(gameObject *object, char filename[]){
	FILE *gameData;
	gameData = fopen(filename,"r");
	
	if(gameData == NULL){
		perror("File opening failed:\n");
		exit(EXIT_FAILURE);
	}


	fscanf(gameData,"%d",&object->number_of_polygons);

	object->x = (double**)malloc(sizeof(double*)*object->number_of_polygons);
	object->y = (double**)malloc(sizeof(double*)*object->number_of_polygons);
	object->size = (int *)malloc(sizeof(int)*object->number_of_polygons);
	object->color = (int (*)[3])malloc(sizeof(int [3])*object->number_of_polygons);

	for(int i = 0; i<object->number_of_polygons; i++){

		fscanf(gameData,"%d",&object->size[i]);
		fscanf(gameData,"%d %d %d",&object->color[i][0],&object->color[i][1],&object->color[i][2]);

		object->x[i] = (double *)malloc(sizeof(double)*object->size[i]);
		object->y[i] = (double *)malloc(sizeof(double)*object->size[i]);

		for(int j = 0; j<object->size[i]; j++){
			fscanf(gameData,"%lf",&object->x[i][j]);
		}
		for(int j = 0; j<object->size[i]; j++){
			fscanf(gameData,"%lf",&object->y[i][j]);
		}

	}
	fclose(gameData);

}


void Draw_gameObject(object_properties player){


	for(int i = 0; i<player.object.number_of_polygons; i++){
		iSetColor(player.object.color[i][0],player.object.color[i][1],player.object.color[i][2]);
		double X[player.object.size[i]];
		double Y[player.object.size[i]];
		
		for(int j = 0; j<player.object.size[i]; j++){

			//converting cartesian coordinate to polar coordinate
			double r = sqrt(player.object.x[i][j]*player.object.x[i][j] + player.object.y[i][j]*player.object.y[i][j]);
			double theta = atan2(player.object.y[i][j],player.object.x[i][j]);

			//modifying angle and converting back to cartesian
			X[j] = player.position.x + r*cos(theta+player.angle);
			Y[j] = player.position.y + r*sin(theta+player.angle);
		}
		iFilledPolygon(X,Y,player.object.size[i]);

	}
}


void start(){
	player.position.x = 500;
	player.position.y = 500;
	player.object = spaceship;
}

void update(){
	//updating the position of the player
	player.position.x += player.velocity.x*dt;
	player.position.y += player.velocity.y*dt;

	//adding friction
	double velocity_magnitude = sqrt(player.velocity.x*player.velocity.x+player.velocity.y*player.velocity.y);
	if(abs(player.velocity.x)>0){
		player.velocity.x -= player.velocity.x/velocity_magnitude*friction*dt;
	}
	if(abs(player.velocity.y)>0){
		player.velocity.y -= player.velocity.y/velocity_magnitude*friction*dt;
	}

	//updating flare particles
	for(int i = 0; i<total_particle; i++){
		
		if(flare[i].life>0.0000001){
			flare[i].life -= dt;
			flare[i].position.x += flare[i].velocity.x*dt;
			flare[i].position.y += flare[i].velocity.y*dt;
		}

	}

}

void thrust(){
	double velocity_magnitude = sqrt(player.velocity.x*player.velocity.x+player.velocity.y*player.velocity.y);
	if(velocity_magnitude<max_velocity){
		player.velocity.x = player.velocity.x + acceleration*cos(player.angle)*dt;
		player.velocity.y = player.velocity.y + acceleration*sin(player.angle)*dt;
	}
	
	double adjust = 25;
	int cnt = 0;
	int active = 0;
	for(int i = 0; i<1000; i++){
		if(flare[i].life<0.00000001){
			cnt++;
			flare[i].life = particle_life;
			double velocity = rand()%1000;
			flare[i].velocity.x = -velocity*cos(player.angle);
			flare[i].velocity.y = -velocity*sin(player.angle);
			flare[i].position.x = player.position.x-adjust*cos(player.angle);
			flare[i].position.y = player.position.y-adjust*sin(player.angle);
		}       
		else active++;
		if(cnt>20)break;
	}
	printf("Active: %d\n",active);
}

void Draw_flare(){
	iSetColor(18, 206, 219);
	for(int i = 0; i<total_particle; i++){
		if(flare[i].life>0.00001) iFilledCircle(flare[i].position.x,flare[i].position.y,((rand()%1000)/100.0f)*(flare[i].life/particle_life));
	}
}