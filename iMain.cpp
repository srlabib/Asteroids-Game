#include "iGraphics.h"
#include <stdlib.h>
#include "structures.cpp"
#include <time.h>

//#define DEBUG

int wave = 1;
bool gameover = 0;
//spaceship properties
gameObject spaceship;
object_properties player;
const double dt = 0.005;                   
const double acceleration = 20000;
const double max_velocity = 1200;
const double friction = 1500;
bool actv_pl_en_col = 0;


//ENEMY
const int max_enemy = 10;
const double reload_time = 1000;
int active_enemy = 1;
gameObject enemy[3];
object_properties enemy_porperties[max_enemy];
vector2 enemy_destination[max_enemy];
double enemy_direction[max_enemy];
double enemy_reload_time[max_enemy];


//particle
const int total_particle = 200;
thrust_particle flare[total_particle];
const double particle_life = 0.05;
double particle_initial_pos = 30;
int flare_intensity = 20;
const int total_exp_particle = 10000;
explosion_particle explosion[total_exp_particle];
int active_exp_particle = 0; //all particles with index smaller than the value is active

//bullets
const int max_bullet = 20;
bullet bullets[max_bullet];
double bullet_velocity = 4000;
//enemy  bullet
bullet enemy_bullets[max_bullet];


//asteroidss
const int max_asteroids = 1000;
gameObject asteroids[3];
object_properties asteroids_properties[max_asteroids];
double asteroid_scale1 = 0.7;
double asteroid_scale2 =0.5;
double asteroid_scale3 = 0.3;



int active_ateroids = 0;
int asteroid_limit = 10;

//environment
const int num_of_stars = 800;
vector2 stars_pos1[num_of_stars/2];
vector2 stars_pos2[num_of_stars/2];
double world_limit_x = 3000;
double world_limit_y = 1687;

vector2 camera_offset = {0,0};
double cam_factor_x,cam_factor_y;


void inititalize_gameObjects(gameObject *object, char filename[]);
void Draw_gameObject(object_properties player);
void generate_asteroid();
void create_asteroids(vector2 position, double scale);
void Draw_flare();
void Draw_bullet(bullet bullets[]);
void thrust();
void fire();
void update();
void start();
bool isColliding(object_properties object1, object_properties object2);
void Destroy_asteroid(int index);
void ControlEnemy();
void SendEnemy(int level);
void destroy_enemy(int index);
void enemy_attack();
double distance(vector2 a, vector2 b);
void draw_enemy_health_bar();
void explode(vector2 pos,vector2 velocity,bool collision);  //collision = 1 when just colliding not exploding
void draw_explosion();


void iDraw() {
	
	iClear();

	//the sky
	iSetColor(11, 0, 36);
	iFilledRectangle(0,0,1280,720);
	iSetColor(255,255,255);	
	// stars
	for(int i = 0; i<num_of_stars/2; i++){
		iPoint(stars_pos1[i].x-camera_offset.x*0.7,stars_pos1[i].y-camera_offset.y*0.7,(rand()%100)*.03);
		iPoint(stars_pos2[i].x-camera_offset.x*0.5,stars_pos2[i].y-camera_offset.y*0.5,(rand()%100)*.015);
	}
	
	
	for(int i = 0; i<active_ateroids; i++){
		Draw_gameObject(asteroids_properties[i]);
	}
	Draw_flare();
	Draw_bullet(bullets);
	Draw_bullet(enemy_bullets);
	Draw_gameObject(player);
	for(int i = 0; i<active_enemy; i++){
		Draw_gameObject(enemy_porperties[i]);
	}
	draw_enemy_health_bar();
	draw_explosion();

	#ifdef DEBUG
	char text[50];
	sprintf(text,"%lf %lf %lf",player.position.x,player.position.y,player.angle);
	iSetColor(255,255,255);
	iText(player.position.x-camera_offset.x-50,player.position.y-camera_offset.y-50,text);
	for(int i = 0; i<active_enemy; i++){
		iLine(enemy_porperties[i].position.x-camera_offset.x,enemy_porperties[i].position.y-camera_offset.y,enemy_destination[i].x-camera_offset.x,enemy_destination[i].y-camera_offset.y);
	}
	#endif // DEBUG

	char health[50];
	sprintf(health, "Health :%g",player.life);
	iText(1100,700,health,GLUT_BITMAP_HELVETICA_18);

	if(gameover){
		iText(610,360,"GAME OVER!",GLUT_BITMAP_HELVETICA_18);
		iDelay(2);
		start();
	}
	
}


/*
	function iMouseMove() is called when the user presses and drags the mouse.
	(mx, my) is the position where the mouse pointer is.
*/
void iMouseMove(int mx, int my) {
	

}

void iPassiveMouseMove(int mx, int my){
	player.angle = atan2(my-player.position.y+camera_offset.y,mx-player.position.x+camera_offset.x);
}

/*
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouse(int button, int state, int mx, int my) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		fire();
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
	if(key == 'f')fire();
	if(key == ' ')thrust();
	if(key == 'p')iPauseTimer(0);

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
	srand(time(0));
	
	inititalize_gameObjects(&spaceship,"assets/spaceship.txt");
	inititalize_gameObjects(&asteroids[0],"assets/asteroid1.txt");
	inititalize_gameObjects(&asteroids[1],"assets/asteroid2.txt");
	inititalize_gameObjects(&asteroids[2],"assets/asteroid3.txt");


//Testing enemy sprite
	inititalize_gameObjects(&enemy[0],"assets/enemy1.txt");
	inititalize_gameObjects(&enemy[1],"assets/enemy1.txt");
	inititalize_gameObjects(&enemy[2],"assets/enemy1.txt");
	inititalize_gameObjects(&enemy[3],"assets/enemy1.txt");
	

//testing enemy sprite

	start();
	iSetTimer(dt*1000,update);
	iInitialize(1280, 720, "asteroids Game");
	

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
	fscanf(gameData,"%lf",&object->collider_radius);
	fclose(gameData);

}

void Draw_gameObject(object_properties player){


	for(int i = 0; i<player.object.number_of_polygons; i++){

		double X[player.object.size[i]];
		double Y[player.object.size[i]];
		
		for(int j = 0; j<player.object.size[i]; j++){

			//converting cartesian coordinate to polar coordinate
			double r = sqrt(player.object.x[i][j]*player.object.x[i][j]*player.scale*player.scale + player.object.y[i][j]*player.object.y[i][j]*player.scale*player.scale);
			double theta = atan2(player.object.y[i][j],player.object.x[i][j]);

			//modifying angle and converting back to cartesian
			X[j] = player.position.x + r*cos(theta+player.angle)-camera_offset.x;
			Y[j] = player.position.y + r*sin(theta+player.angle)-camera_offset.y;

		}
		iSetColor(player.object.color[i][0],player.object.color[i][1],player.object.color[i][2]);
		iFilledPolygon(X,Y,player.object.size[i]);

		#ifdef DEBUG
		for(int j = 0; j<player.object.size[i]; j++){
			iSetColor(255, 17, 0);
			iPoint(X[j],Y[j],2);	
		}
		#endif // DEBUG

	}
	#ifdef DEBUG
	iSetColor(255,255,255);
	iCircle(player.position.x-camera_offset.x,player.position.y-camera_offset.y,player.object.collider_radius*player.scale);
	
	#endif // DEBUG
	
}

void Draw_flare(){
	iSetColor(18, 206, 219);
	for(int i = 0; i<total_particle; i++){
		if(flare[i].life>0.00001) iFilledCircle(player.position.x-flare[i].position*cos(player.angle)-camera_offset.x,player.position.y-flare[i].position*sin(player.angle)-camera_offset.y,((rand()%1000)/100.0f)*(flare[i].life/particle_life)*player.scale);
	}
}

void Draw_bullet(bullet bulletss[]){
	iSetColor(247, 49, 5);
	for(int i = 0; i<max_bullet; i++){
		if(bulletss[i].active){
			iFilledCircle(bulletss[i].position.x-camera_offset.x,bulletss[i].position.y-camera_offset.y,5);
		}
	}
}

void start(){
	player.position.x = 0;
	player.position.y = 0;
	player.velocity.x = 0;
	player.velocity.y = 0;
	player.object = spaceship;
	player.scale = 0.7;
	player.life = 100;

	cam_factor_x = (world_limit_x/2-640.0)/(world_limit_x/2);
	cam_factor_y = (world_limit_y/2-320.0)/(world_limit_y/2);

	//setting random postion of the stars
	for(int i = 0; i<num_of_stars/2; i++){
		stars_pos1[i].x = -world_limit_x/2+rand()%(int)world_limit_x;
		stars_pos1[i].y = -world_limit_y/2+rand()%(int)world_limit_y;
		stars_pos2[i].x = -world_limit_x/2+rand()%(int)world_limit_x;
		stars_pos2[i].y = -world_limit_y/2+rand()%(int)world_limit_y;
	}

	//reseting all variables
	active_ateroids = 0;
	active_enemy = 0;
	for(int i = 0; i<max_bullet; i++){
		bullets[i].active = 0;
		enemy_bullets[i].active = 0;
	}
	gameover = 0;
	wave = 1;
	SendEnemy(1);


}

void update(){

	// updating the position of the camera
	camera_offset.x = player.position.x*cam_factor_x-640;
	camera_offset.y = player.position.y*cam_factor_y-360;


	//updating the position of the player
	player.position.x += player.velocity.x*dt;
	player.position.y += player.velocity.y*dt;

	// limiting spaceship movement within world limit
	if(player.position.x-camera_offset.x<0){
		player.position.x = camera_offset.x+20;
		player.velocity.x += 500;
	}
	if(player.position.x-camera_offset.x>1280){
		player.position.x = camera_offset.x+1275;
		player.velocity.x -= 500;
	}
	if(player.position.y-camera_offset.y<0){
		player.position.y = camera_offset.y+10;
		player.velocity.y += 500;
	}
	if(player.position.y-camera_offset.y>720){
		player.position.y = camera_offset.y+715;
		player.velocity.y -= 500;
	}

	//adding friction
	double velocity_magnitude = sqrt(player.velocity.x*player.velocity.x+player.velocity.y*player.velocity.y);
	if(abs(player.velocity.x)>0){
		player.velocity.x -= player.velocity.x/velocity_magnitude*friction*dt;
	}
	if(abs(player.velocity.y)>0){
		player.velocity.y -= player.velocity.y/velocity_magnitude*friction*dt;
	}

	//updating flare particles
	int active = 0;
	for(int i = 0; i<total_particle; i++){
		
		if(flare[i].life>0.0000001){
			flare[i].life -= dt;
			flare[i].position += flare[i].velocity*dt;
			active++;
		}
	}

	//updating explostion particles
	for(int i = 0; i<active_exp_particle; i++){
		explosion[i].life -= dt;
		if(explosion[i].life<=0){
			explosion[i] = explosion[--active_exp_particle];
		}
		explosion[i].position.x += explosion[i].velocity.x*dt;
		explosion[i].position.y += explosion[i].velocity.y*dt;

		explosion[i].velocity.x /= 1.01;
		explosion[i].velocity.y /= 1.01;

	}
	//	printf("Active: %d\t",active);


	//updating bullets
	active = 0;
	for(int i = 0; i<max_bullet; i++){
		if(bullets[i].active){
			bullets[i].position.x += bullets[i].velocity.x*dt;
			bullets[i].position.y += bullets[i].velocity.y*dt;

			if(bullets[i].position.x-camera_offset.x<0|| bullets[i].position.x-camera_offset.x>1280 || bullets[i].position.y-camera_offset.y<0||bullets[i].position.y-camera_offset.y>720){
				bullets[i].active = 0;
			}

		}
	}
	//updating enemy bullets
	for(int i = 0; i<max_bullet; i++){
		if(enemy_bullets[i].active){
			enemy_bullets[i].position.x += enemy_bullets[i].velocity.x*dt;
			enemy_bullets[i].position.y += enemy_bullets[i].velocity.y*dt;

			if(enemy_bullets[i].position.x-camera_offset.x<0|| enemy_bullets[i].position.x-camera_offset.x>1280 || enemy_bullets[i].position.y-camera_offset.y<0||enemy_bullets[i].position.y-camera_offset.y>720){
				enemy_bullets[i].active = 0;
			}

		}
	}


	//updating asteroids
	while(active_ateroids<asteroid_limit){
		generate_asteroid();
	}
	for(int i = 0; i<active_ateroids; i++){
		if(-world_limit_y/2-10> asteroids_properties[i].position.y || asteroids_properties[i].position.y>world_limit_y/2+10 || -world_limit_x/2-10> asteroids_properties[i].position.x || asteroids_properties[i].position.x>world_limit_x/2+10){
			asteroids_properties[i] = asteroids_properties[--active_ateroids];
		}
		asteroids_properties[i].angle += asteroids_properties[i].angular_velocity*dt;
		if(asteroids_properties[i].angle>=acos(-1)*2)asteroids_properties[i].angle = 0;
		if(asteroids_properties[i].angle<0)asteroids_properties[i].angle = acos(-1)*2-0.00001;
		asteroids_properties[i].position.x+=asteroids_properties[i].velocity.x*dt;
		asteroids_properties[i].position.y+=asteroids_properties[i].velocity.y*dt;
		
	}
	//printf("%d\n",active_ateroids);


	// asteroid player collision
	for(int i = 0; i<active_ateroids; i++){
		if(isColliding(player,asteroids_properties[i])){
			//iPauseTimer(0);
			player.life -= 10*asteroids_properties[i].scale;
			explode(asteroids_properties[i].position,{0,0},0);
			Destroy_asteroid(i);
			
		}
	}

	// player enemy collision
	for(int i = 0; i<active_enemy; i++){
		if(isColliding(player,enemy_porperties[i])){
			if(actv_pl_en_col){
				player.life -= 15;
				actv_pl_en_col = 0;
			}
			destroy_enemy(i);
		}
		else actv_pl_en_col = 1;
	}

	//player enemy_bullet collision
	for(int i = 0; i < max_bullet; i++){
		if(!enemy_bullets[i].active)continue;
		if(distance(player.position,enemy_bullets[i].position)<player.object.collider_radius){
			player.life -= 5;
			explode(enemy_bullets[i].position,{0,0},1);
			enemy_bullets[i].active = 0;
		}
	}

	// asteroids+enemy bullet collision
	for(int i = 0; i<20; i++){
		if(!bullets[i].active)continue;
		for(int j = 0; j<active_ateroids; j++){
			if(distance(asteroids_properties[j].position,bullets[i].position)<asteroids_properties[j].object.collider_radius*asteroids_properties[j].scale)
			{
				if(asteroids_properties[j].scale == asteroid_scale1){
					create_asteroids(asteroids_properties[j].position,asteroid_scale2);
					create_asteroids(asteroids_properties[j].position,asteroid_scale2);
				}
				else if(asteroids_properties[j].scale == asteroid_scale2){
					create_asteroids(asteroids_properties[j].position,asteroid_scale3);
					create_asteroids(asteroids_properties[j].position,asteroid_scale3);
					create_asteroids(asteroids_properties[j].position,asteroid_scale3);
				}
				Destroy_asteroid(j);
				explode(bullets[i].position,{0,0},1);
				bullets[i].active = 0;
				
			}
		}
		for(int j = 0; j<active_enemy; j++){
			if(distance(enemy_porperties[j].position,bullets[i].position)<enemy_porperties[j].object.collider_radius*enemy_porperties[j].scale)
			{
				enemy_porperties[j].life -= 20;
				bullets[j].active = 0;
				explode(bullets[i].position,{0,0},1);
			}
			if(enemy_porperties[j].life <= 0){
				destroy_enemy(j);
			}
		}
	}

	ControlEnemy();
	enemy_attack();

	if(active_enemy == 0){
		SendEnemy(++wave);
	}

	if(player.life<=0){
		gameover = 1;

	}

}

void thrust(){
	double velocity_magnitude = sqrt(player.velocity.x*player.velocity.x+player.velocity.y*player.velocity.y);
	if(velocity_magnitude<max_velocity){
		player.velocity.x = player.velocity.x + acceleration*cos(player.angle)*dt;
		player.velocity.y = player.velocity.y + acceleration*sin(player.angle)*dt;
	}
	
	int cnt = 0;
	for(int i = 0; i<total_particle; i++){
		if(flare[i].life<0.00000001){
			cnt++;
			flare[i].life = particle_life;
			flare[i].velocity = rand()%1000;
			flare[i].position = particle_initial_pos*player.scale;
		}       

		if(cnt>flare_intensity)break;
	}
}



void fire(){

	for(int i = 0; i<max_bullet; i++){
		if(!bullets[i].active){
			bullets[i].position = player.position;
			bullets[i].velocity.x = bullet_velocity*cos(player.angle);
			bullets[i].velocity.y = bullet_velocity*sin(player.angle);
			bullets[i].active = 1;
			break;
		}

	}
}

//Generates asteroids automatically on the edge of the game world when asteroid count is less than max count
void generate_asteroid(){
	enum side x = static_cast<side>(rand()%4);
	double pos;
	double angle;
	switch (x)
	{
	case bottom:
		pos = -world_limit_x/2+rand()%(int)world_limit_x;
		asteroids_properties[active_ateroids].position.x = pos;
		asteroids_properties[active_ateroids].position.y = -world_limit_y/2;
		angle = 60+rand()%60;
		break;
	
	case top:
		pos = -world_limit_x/2+rand()%(int)world_limit_x;
		asteroids_properties[active_ateroids].position.x = pos;
		asteroids_properties[active_ateroids].position.y = world_limit_y/2;
		angle = -60-rand()%60;
		break;

	case left:
		pos = -world_limit_y/2+rand()%(int)world_limit_y;
		asteroids_properties[active_ateroids].position.x = -world_limit_x/2;
		asteroids_properties[active_ateroids].position.y = pos;
		angle = -30+rand()%60;
		break;

	case right:
		pos = -world_limit_y/2+rand()%(int)world_limit_y;
		asteroids_properties[active_ateroids].position.x = world_limit_x/2;
		asteroids_properties[active_ateroids].position.y = pos;
		angle = 150+rand()%60;
		break;
	}
	

	double velocity = 500+rand()%500;
	asteroids_properties[active_ateroids].velocity.x = velocity*cos(angle*acos(-1)/180);
	asteroids_properties[active_ateroids].velocity.y = velocity*sin(angle*acos(-1)/180);
	asteroids_properties[active_ateroids].angular_velocity = -10 + (double)(rand()%2000)/100.0;
	asteroids_properties[active_ateroids].life = 100;
	asteroids_properties[active_ateroids].scale = asteroid_scale1;
	asteroids_properties[active_ateroids].object = asteroids[rand()%3];
	active_ateroids++;
}

// This function creates asteroids on a specific location (where a bigger asteroid is broken)
void create_asteroids(vector2 position, double scale){
	asteroids_properties[active_ateroids].position = position;
	double angle = rand()%360;
	double velocity = 500+rand()%500;
	asteroids_properties[active_ateroids].velocity.x = velocity*cos(angle*acos(-1)/180);
	asteroids_properties[active_ateroids].velocity.y = velocity*sin(angle*acos(-1)/180);
	asteroids_properties[active_ateroids].angular_velocity = -10 + (double)(rand()%2000)/100.0;
	asteroids_properties[active_ateroids].life = 100;
	asteroids_properties[active_ateroids].scale = scale;
	asteroids_properties[active_ateroids].object = asteroids[rand()%3];
	active_ateroids++;
}

bool isColliding(object_properties object1, object_properties object2){
	double dx = (object1.position.x-object2.position.x);
	double dy = (object1.position.y-object2.position.y);
	double r2 = object1.object.collider_radius*object1.scale+object2.object.collider_radius*object2.scale;


	if(dx*dx + dy*dy < r2*r2) return 1;
	return 0;
}

void Destroy_asteroid(int index){
	asteroids_properties[index] = asteroids_properties[active_ateroids-1];
	active_ateroids--;
}


void ControlEnemy(){
	for(int i = 0; i<active_enemy; i++){
		if(distance(enemy_destination[i],enemy_porperties[i].position)<300){
			enemy_destination[i].x = camera_offset.x+rand()%1280;
			enemy_destination[i].y = camera_offset.y+rand()%720;
		}
		enemy_direction[i] = atan2(-enemy_porperties[i].position.y+enemy_destination[i].y,-enemy_porperties[i].position.x+enemy_destination[i].x);
		if(enemy_direction[i]<0)enemy_direction[i]+=acos(-1)*2;
		if(abs(enemy_direction[i]-enemy_porperties[i].angle)<0.05)enemy_porperties[i].angle = enemy_direction[i];
		if(enemy_porperties[i].angle<enemy_direction[i]){
			if(enemy_direction[i]-enemy_porperties[i].angle<acos(-1)*2-(enemy_direction[i]-enemy_porperties[i].angle)){
				enemy_porperties[i].angle+=0.05;
			}else{
				enemy_porperties[i].angle-=0.05;
			}
		}else{
			if(enemy_porperties[i].angle-enemy_direction[i]<acos(-1)*2-(enemy_porperties[i].angle-enemy_direction[i])){
				enemy_porperties[i].angle-=0.05;
			}else{
				enemy_porperties[i].angle+=0.05;
			}
		}
		if(enemy_porperties[i].angle<0)enemy_porperties[i].angle = acos(-1)*2;
		else if(enemy_porperties[i].angle>acos(-1)*2)enemy_porperties[i].angle =0;
		
		enemy_porperties[i].velocity.x = max_velocity*cos(enemy_porperties[i].angle);
		enemy_porperties[i].velocity.y = max_velocity*sin(enemy_porperties[i].angle);
		enemy_porperties[i].position.x += enemy_porperties[i].velocity.x*dt;
		enemy_porperties[i].position.y += enemy_porperties[i].velocity.y*dt;
	}
	
}
void SendEnemy(int wave){
	active_enemy = wave;
	for(int i = 0; i<active_enemy; i++){
		enemy_porperties[i].object = enemy[rand()%3];
		enemy_porperties[i].life = 100;
		enemy_porperties[i].scale = 0.4;
		enum side x = static_cast<side>(rand()%4);
		switch (x)
		{
		case bottom:
			enemy_porperties[i].position.x = 0;
			enemy_porperties[i].position.y = -world_limit_y*2;
			break;
		case top:
			enemy_porperties[i].position.x = 0;
			enemy_porperties[i].position.y = world_limit_y*2;
			break;
		case left:
			enemy_porperties[i].position.x = -world_limit_x*2;
			enemy_porperties[i].position.y = 0;
			break;
		case right:
			enemy_porperties[i].position.x = world_limit_x*2;
			enemy_porperties[i].position.y = 0;
			break;
		
		default:
			break;
		}
		enemy_destination[i] = enemy_porperties[i].position;
		enemy_reload_time[i] = rand()%(int)reload_time;
	}
}
void destroy_enemy(int index){
	explode(enemy_porperties[index].position,enemy_porperties[index].velocity,0);
	enemy_porperties[index] = enemy_porperties[active_enemy-1];
	enemy_destination[index] = enemy_destination[active_enemy-1];
	enemy_reload_time[index] = enemy_reload_time[active_enemy-1];
	enemy_direction[index] = enemy_direction[active_enemy-1];
	active_enemy--;
}
double distance(vector2 a, vector2 b){
	return sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
}
void enemy_attack(){
	for(int i = 0; i<active_enemy; i++){
		if(enemy_reload_time[i]>0.00001){
			enemy_reload_time[i] -= 5;
			continue;
		}

		double enemy_player_angle = atan2(player.position.y-enemy_porperties[i].position.y,player.position.x-enemy_porperties[i].position.x);

		for(int j = 0; j<max_bullet; j++){
			if(enemy_bullets[j].active)continue;
			enemy_bullets[j].position = enemy_porperties[i].position;
			enemy_bullets[j].velocity.x = bullet_velocity*cos(enemy_player_angle);
			enemy_bullets[j].velocity.y = bullet_velocity*sin(enemy_player_angle);
			enemy_bullets[j].active = 1;
			enemy_reload_time[i] = reload_time;
			break;
		}
	}
}

void draw_enemy_health_bar(){
	for(int i = 0; i<active_enemy; i++){
		iSetColor(255,255,255);
		iRectangle(enemy_porperties[i].position.x-camera_offset.x-40,enemy_porperties[i].position.y-camera_offset.y+60,80,5);
		iSetColor(207, 2, 2);
		iFilledRectangle(enemy_porperties[i].position.x-camera_offset.x-40,enemy_porperties[i].position.y-camera_offset.y+60,80*enemy_porperties[i].life/100,5);
	}
}


void explode(vector2 pos,vector2 velocity,bool collision){
	int particle_count = 200;
	if(collision)particle_count = 30;
	for(int i = 0; i<particle_count; i++){
		double r = rand()%30;
		double angle = rand()%360;
		explosion[active_exp_particle].position.x = pos.x+r*cos(angle);
		explosion[active_exp_particle].position.y = pos.y+r*sin(angle);
		explosion[active_exp_particle].velocity.x = velocity.x/5;
		explosion[active_exp_particle].velocity.y = velocity.y/5;

		double v;
		if(collision){
			v = rand()%500;
			explosion[active_exp_particle].radius = 2;
		}
		else{
			if(i<130 ){
				v = rand()%50;
				explosion[active_exp_particle].radius = 10;
			}
			else{
				v = 500+rand()%500;
				explosion[active_exp_particle].radius = 1.5;
			}
		}
		explosion[active_exp_particle].angle = angle*acos(-1)/180;
		explosion[active_exp_particle].velocity.x += v*cos(angle);
		explosion[active_exp_particle].velocity.y += v*sin(angle);
		explosion[active_exp_particle].life = 0.2;
		active_exp_particle++;
	}
}



void draw_explosion(){
	iSetColor(255,255,255);
	for(int i = 0; i<active_exp_particle; i++){
		iFilledCircle(explosion[i].position.x-camera_offset.x,explosion[i].position.y-camera_offset.y,(explosion[i].radius)*(explosion[i].life/0.2));
	}
}