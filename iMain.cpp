#include "iGraphics.h"
#include <stdlib.h>
#include "structures.cpp"
#include <time.h>
#include <windows.h>

//#define DEBUG
//#define DEV_MODE

int wave = 1;
bool gameover = 0;
bool multiplayer = 0;
page state;
int game_clock;  // timer for main game
int menu_clock;  // timer for menu and others
clock_t startTime_explosion,saveTime,currTime;

FILE *saved_data;
FILE *High_score_Data;
bool saved_game_available = 0;

//MainMenu Stuffs
Image MainMenuBG;
Image GameLogo;
vector2 buttons[5] = {{450,400},{450,310},{450,220},{450,130},{450,40}}; //MainMenu Buttons position
int buttonHeight = 80, buttonWidth = 375;
Image ButtonImage[6];
Image ButtonHighligter;
vector2 ButtonHighlighterpos = {-1,-1};
Image choose_screen_multi;
Image choose_screen_single;
Image TextBox;
Image aboutPage;
bool paused = 0;

//UI
Image GameOverText;
Image PausedText;
Image count_digit[3];

double countdown = 0;

//spaceship properties
gameObject spaceship[3];			//Polygons of the spaceships
object_properties player,player2;
const double dt = 0.005;
const double acceleration = 20000;
const double max_velocity = 1200;
const double friction = 1500;
bool actv_pl_en_col = 1;
const double player_rt = 0.04;//player reload time
double reloadTime_p1 = player_rt,reloadTime_p2 = player_rt;

//score
int total_score = 0;
int high_score = 0;
int	destroy_count = 0;
int	kill_count = 0;
double messageTimer = 0;
int msg=0;
//player2 score (multiplayer)
int total_score_p2 = 0;
int	destroy_count_p2 = 0;
int	kill_count_p2 = 0;


//ENEMY
const int max_enemy = 10;
const double reload_time = 500; // enemy realod time
int active_enemy = 1;
gameObject enemy[3];
object_properties enemy_porperties[max_enemy];
vector2 enemy_destination[max_enemy];   //coordinate of destinatoin of each enemy chosen randomly
double enemy_direction[max_enemy];      //current direction of enenmy (toward destination)
double enemy_reload_time[max_enemy];


//particle
const int total_particle = 400;
thrust_particle flare[total_particle];                //particles for spaceship thrust
const double particle_life = 0.05;
const double particle_initial_pos = 30;
const int flare_intensity = 20;						// number of particle created each call of thrust function
const int total_exp_particle = 10000;
explosion_particle explosion[total_exp_particle];    //particles for explosion
int active_exp_particle = 0;                         //all particles with index smaller than the value is active

//bullets
const int max_bullet = 20;
bullet bullets[max_bullet];
double bullet_velocity = 4000;
gameObject bullet_obj;
//enemy  bullet
bullet enemy_bullets[max_bullet];


//asteroidss
const int max_asteroids = 1000;
gameObject asteroids[3];
object_properties asteroids_properties[max_asteroids];
double asteroid_scale1 = 0.7;
double asteroid_scale2 =0.5;
double asteroid_scale3 = 0.3;


//multiplayer mode
const vector2 player1_initital_pos = {-426,0},player2_inititial_pos = {+426,0};
bool p1_thrust_toggle = 0;
bool p2_thrust_toggle = 0;
double turn_p1 = 0,turn_p2 = 0;
bool p1_Dead = 0,p2_Dead = 0;

char name_p1[30],name_p2[30];
int editing = -1,in1,in2;
int spaceship_in1,spaceship_in2;

int soundcount = 0;
int max_sound = 50;

int active_ateroids = 0;
int asteroid_limit = 10;

//environment
const int num_of_stars = 400;
vector2 stars_pos1[num_of_stars/2];
vector2 stars_pos2[num_of_stars/2];
double world_limit_x = 3000;
double world_limit_y = 1687;

vector2 camera_offset = {0,0};
double cam_factor_x,cam_factor_y,shake_factor = 0,shakeTime = 500;

void initializeGameobject(gameObject *object, char filename[]);
void Draw_MainGame();
void Draw_mainMenu();
void Draw_gameObject(object_properties player);
void generate_asteroid();
void create_asteroids(vector2 position, double scale);
void Draw_flare();
void Draw_bullet(bullet bullets[]);
void thrust(object_properties *);
void fire(object_properties *);
void update_gameplay();
void start(int mode);
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
void Load_resources();
void draw_gameover_screen();
void draw_pause_screen();
void SaveGame();
void LoadGame();
void player_movement(object_properties *player);
void player_collision(object_properties *player);
void Draw_chooseScreen();
int is_file_empty(FILE *file);
void playsound(char filename[]);


void iDraw() {
	
	iClear();

	if(state == MainMenu){
		Draw_mainMenu();
	}
	if(state == Game){
		Draw_MainGame();
	}
	if(state == choose){
		Draw_chooseScreen();
	}
	if(state == AboutPage){
		iShowImage(1,1,&aboutPage);
	}
	
}

void iMouseMove(int mx, int my) {}

void iPassiveMouseMove(int mx, int my){
	//Updating the direction of the spaceship toward the mouse pointer
	if(!multiplayer && state == Game &&!paused && countdown <=0){
		player.angle = atan2(my-player.position.y+camera_offset.y,mx-player.position.x+camera_offset.x);
	}
	
	if(state == MainMenu){   //Setting up mainMenu Button highlighter position in case of hovering
		for(int i = 0;i < 5; i++){
			if(buttons[i].x<=mx && mx<= buttons[i].x+buttonWidth && buttons[i].y<=my && my<= buttons[i].y+buttonHeight){
				ButtonHighlighterpos = buttons[i];
				break;
			}
			else{
				ButtonHighlighterpos = {-1,-1};
			}
		}
	}
	if(state == Game && gameover){ // setting up gameover screen buttons position in case of hovering
		if(40<=mx && mx<=160 && 60<=my && my <= 100){
			ButtonHighlighterpos = {30,35}; // highlighter on the try again button
		}
		else if(1070<=mx && mx<=1201 && 60<=my && my <= 100){
			ButtonHighlighterpos = {1070,35}; // highlighter on the mainmenu button
		}
		else{
			ButtonHighlighterpos = {-1,-1}; // inactivating highlighter 
		}
	}
	
}

/*
	function iMouse() is called when the user presses/releases the mouse.
	(mx, my) is the position where the mouse pointer is.
	*/
void iMouse(int button, int stat, int mx, int my) {

	if(!multiplayer && state == Game && !paused && countdown<=0){
		if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN) {
			/*if(reloadTime_p1<=0)*/ fire(&player); //firing of left mouse click
		}
		if (button == GLUT_RIGHT_BUTTON && stat == GLUT_DOWN) {

		}
	}
	if(state == Game && gameover){ // mouse control when on gameover screen
		if(ButtonHighlighterpos.x == 30 && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			if(multiplayer)start(multiplayer_mode);   // restarting game clicking on try again button
			else start(Game);
		}
		if(ButtonHighlighterpos.x == 1070 && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			iPauseTimer(game_clock);
			state = MainMenu;  //going to main menu clicking on main menu button
			PlaySound(TEXT("assets/Sound/bgm.wav"),NULL,SND_ASYNC | SND_LOOP);
			iLoadImage(&ButtonHighligter,"assets/Buttons/HoverBar.png");
			if(!multiplayer){
				saved_data = fopen("GameData/gamedata.bin","wb");
				saved_game_available = 0;
				fwrite(&saved_game_available,sizeof(bool),1,saved_data); //erasing previous saved game data
				fclose(saved_data);
				saved_data = fopen("GameData/gamedata.bin","rb");
				if(fread(&saved_game_available,sizeof(bool),1,saved_data)!=1){
					printf("Error reADING DATA\n");
				}
			}
		}
	}
	if(state == MainMenu){  //mouse control when on the mainmenu
		//Presing the start game button
		if(ButtonHighlighterpos.y == buttons[startgame].y && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			state = choose; // choosing the spaceship page
			multiplayer = 0;
			spaceship_in1 = 0;
			camera_offset = {0,0};
			player.position = {635, 508};   //position of the spaceship where it is dispayed for choosing
			player.angle = 0;
			player.object = spaceship[0];
			player.scale = 0.7;

		}
		// pressing the continue game button
		if(saved_game_available && ButtonHighlighterpos.y == buttons[continuegame].y && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			LoadGame();
			paused = 0;
			gameover = 0;
			multiplayer = 0;
			countdown = 3;
			state = Game;
			// PlaySound(0,0,0);
			ButtonHighlighterpos = {-1,-1};
			iResumeTimer(game_clock);
		}
		//pressing the bultiplayer button
		if(ButtonHighlighterpos.y == buttons[Multi_player].y && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			strcpy(name_p1,"Player 1");
			strcpy(name_p2,"Player 2");
			spaceship_in1 = 0;
			spaceship_in2 = 0;
			in1 = in2 = 8;
			camera_offset = {0,0};
			player.position = {217,357};	//position of the spaceship 1 where it is dispayed for choosing
			player2.position = {1061, 367};	//position of the spaceship 2 where it is dispayed for choosing
			player.object = spaceship[0];
			player2.object = spaceship[0];
			player.angle = 0;
			player2.angle = 0;
			player.scale = 0.7;
			player2.scale = 0.7;
			multiplayer = 1;
			state = choose;
		}

		if(ButtonHighlighterpos.y == buttons[About].y && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			state = AboutPage;
		}
		// pressing exit
		if(ButtonHighlighterpos.y == buttons[exitt].y && button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN){
			exit(0);
		}

	}
	// mouse control on the choosing spcaeship page
	if(state == choose){
		if(multiplayer){ // when its multiplayer mode
			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 81<=mx&&mx<=388&&512<=my&&my<=556)editing = 0;
			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 901<=mx&&mx<=1213&&512<=my&&my<=556)editing = 1;

			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 61<=mx&&mx<=112&&342<=my&&my<=396)player.object = spaceship[spaceship_in1 = (spaceship_in1+2)%3];
			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 349<=mx&&mx<=404&&342<=my&&my<=396)player.object = spaceship[spaceship_in1 = (spaceship_in1+1)%3];

			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 889<=mx&&mx<=933&&342<=my&&my<=396)player2.object = spaceship[spaceship_in2 = (spaceship_in2+2)%3];
			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 1178<=mx&&mx<=1222&&342<=my&&my<=396)player2.object = spaceship[spaceship_in2 = (spaceship_in2+1)%3];
		}
		else{ // when it is single player mode
			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 470<=mx&&mx<=514&&484<=my&&my<=551)player.object = spaceship[spaceship_in1 = (spaceship_in1+2)%3];
			if(button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN && 766<=mx&&mx<=806&&484<=my&&my<=551)player.object = spaceship[spaceship_in1 = (spaceship_in1+1)%3];
		}
	}

}

/*
	function ikeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed.
*/
void iKeyboard(unsigned char key) {
	//Keuboard control while on the signle player gameplay
	if(!multiplayer&&!paused && state == Game ){
		if(key == ' ')thrust(&player);
		if(key == 27){  					// ecape button ASCII = 27 // game paused on pressing esc
			paused = 1;
			iPauseTimer(0);
		}
	}
	if(state == Game && paused){			 // keyboard control when on paused screen
		if(key == 'r'){
			paused = 0;
			iResumeTimer(0);				 //resuming the game
			countdown = 3;
		}
		if(key == 'e'){
			if(!multiplayer){
				SaveGame();					//saving gamedata before exiting
				saved_data = fopen("GameData/gamedata.bin","rb");
				fread(&saved_game_available,sizeof(bool),1,saved_data);
			}
			state = MainMenu;  				// going to main menu
			PlaySound(TEXT("assets/Sound/bgm.wav"),NULL,SND_ASYNC | SND_LOOP);
			iLoadImage(&ButtonHighligter,"assets/Buttons/HoverBar.png");
		}
	}
	// player 1 movement control when on multiplayer mode
	if(multiplayer && player.life>0){		  
		if(key == 'w')p1_thrust_toggle^=1;
		if(key == 'd')turn_p1 = -1;
		if(key == 'a')turn_p1 = 1;
		if(key == ' ')if(reloadTime_p1<=0)fire(&player);
	}
	if(multiplayer){
		if(key == 27){					 // ecape button ASCII = 27 // game paused on pressing esc
			paused = 1;
			iPauseTimer(0);
		}
	}
	//keyboard control when on choosing spaceship screen
	if(state == choose){
		if(key == '\r'){ 			//Pressing enter key starts the game
			if(multiplayer){
				start(multiplayer_mode);
				state = Game;
				ButtonHighlighterpos = {-1,-1};		// making the highlighter inactive
				iResumeTimer(game_clock);
				multiplayer = 1;		
			}else{
				start(Game);
				state = Game;
				ButtonHighlighterpos = {-1,-1};
				iResumeTimer(game_clock);
			}
		}
		else if(editing == 0){			// when the first players name is being edited
			if(key == '\b' && in1 > 0 )in1--;
			else if(20<=key&&key<=126&&in1<28) {
				name_p1[in1++] = key;
			}
		}
		else if(editing == 1){			// when the second players name is being edited
			if(key == '\b' && in2 > 0 )in2--;
			else if(20<=key&&key<=126&&in2<28) {
				name_p2[in2++] = key;
			}
		}
		name_p1[in1] = '\0';
		name_p2[in2] = '\0';
	}

	if(state == AboutPage && key == 27){
		state = MainMenu;
	}



#ifdef DEV_MODE
	if(key == 'x'){
		player.life = 0;
	}
#endif
}
/*
	function iSpecialKeyboard() is called whenver user hits special keys like-function keys, home, end, pg up, pg down, arraows etc. you have to useappropriate constants to detect them.
	A list is:GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
*/
void iSpecialKeyboard(unsigned char key) {
	if(multiplayer && player2.life>0){			// for the player 2 control in multiplayer mode
		if(key == GLUT_KEY_UP)p2_thrust_toggle ^= 1;
		if(key == GLUT_KEY_LEFT)turn_p2 = 1;
		if(key == GLUT_KEY_RIGHT)turn_p2 = -1;
		if(key == GLUT_KEY_HOME)if(reloadTime_p2<=0)fire(&player2);
	}
	if(state == choose && !multiplayer){		// changing the spaceship when on the choosing spacehsip screen
		if(key == GLUT_KEY_LEFT)player.object = spaceship[spaceship_in1 = (spaceship_in1+2)%3];
		if(key == GLUT_KEY_RIGHT)player.object = spaceship[spaceship_in1 = (spaceship_in1+1)%3];
	}
}


int main() {
	srand(time(0));
	//start(Game);
	Load_resources();
	state = MainMenu;
	PlaySound(TEXT("assets/Sound/bgm.wav"),NULL,SND_ASYNC | SND_LOOP);
	game_clock = iSetTimer(dt*1000,update_gameplay);
	iPauseTimer(game_clock);
	
	iInitialize(1280, 720, "AstroStrike - The space defence");
	
	return 0;
}

/* This function Reads the the vertices of the of the gameobject from the txt file(filename)
	and dynamically allocate memory to store the data
*/

void initializeGameobject(gameObject *object, char filename[]){
	FILE *gameData;
	gameData = fopen(filename,"r");
	
	if(gameData == NULL){
		perror("File opening failed:\n");
		exit(EXIT_FAILURE);
	}

	//--text file data format--//
	//Number of polygon
	//for each polygon
	//--number of vetices of the polygon
	//--RGB Color code
	//--verties of x coordinate
	//--Vertices fo y coordinate
	//Collision radius

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

/*  This function draws the iPollygons according to the informations 
	stored in the Object_properties
	This the function that draws all the sprites of the game 
	It gives the advantage to freely rotate the objects and smooth gameperformance despite having too much contents
*/
void Draw_gameObject(object_properties object){

	for(int i = 0; i<object.object.number_of_polygons; i++){

		double X[object.object.size[i]];
		double Y[object.object.size[i]];
		
		for(int j = 0; j<object.object.size[i]; j++){

			//converting cartesian coordinate to polar coordinate w.r.t the center of the object
			double r = sqrt(object.object.x[i][j]*object.object.x[i][j]*object.scale*object.scale + object.object.y[i][j]*object.object.y[i][j]*object.scale*object.scale);
			double theta = atan2(object.object.y[i][j],object.object.x[i][j]);

			//modifying angle and converting back to cartesian
			// as the vertices are w.r.t the center of the object .. rotation will be w.r.t the center
			X[j] = object.position.x + r*cos(theta+object.angle)-camera_offset.x;
			Y[j] = object.position.y + r*sin(theta+object.angle)-camera_offset.y;

		}
		iSetColor(object.object.color[i][0],object.object.color[i][1],object.object.color[i][2]);
		iFilledPolygon(X,Y,object.object.size[i]);

		#ifdef DEBUG
		for(int j = 0; j<object.object.size[i]; j++){
			iSetColor(255, 17, 0);
			iPoint(X[j],Y[j],2);	
		}
		#endif // DEBUG

	}
	#ifdef DEBUG
	iSetColor(255,255,255);
	iCircle(object.position.x-camera_offset.x,object.position.y-camera_offset.y,object.object.collider_radius*object.scale);
	
	#endif // DEBUG
	
}
//Drawing each flare according to the info store is struct flare those are active (life>0)
void Draw_flare(){
	iSetColor(18, 206, 219);
	for(int i = 0; i<total_particle; i++){
		if(flare[i].life>0.00001) iFilledCircle(flare[i].spaceship->position.x-flare[i].position*cos(flare[i].spaceship->angle)-camera_offset.x,flare[i].spaceship->position.y-flare[i].position*sin(flare[i].spaceship->angle)-camera_offset.y,((rand()%1000)/100.0f)*(flare[i].life/particle_life)*flare[i].spaceship->scale);
	}
}
//Drawing each bullet according to the info store is struct bullet those are active
void Draw_bullet(bullet bulletss[]){
	iSetColor(200, 233, 239);
	for(int i = 0; i<max_bullet; i++){
		if(bulletss[i].active){
			// This draws the bullet which is a rectangle 
			double angle = atan2(bulletss[i].velocity.y,bulletss[i].velocity.x);
			double x[4],y[4];
			for(int j = 0; j<4; j++){
				double r = sqrt(bullet_obj.x[0][j]*bullet_obj.x[0][j]+bullet_obj.y[0][j]*bullet_obj.y[0][j]);
				double theta = atan2(bullet_obj.y[0][j],bullet_obj.x[0][j]);
				x[j] = r*cos(theta+angle)+bulletss[i].position.x-camera_offset.x;
				y[j] = r*sin(theta+angle)+bulletss[i].position.y-camera_offset.y;

			}
			iFilledPolygon(x,y,4);
		}
	}
}

/*	This function draws all the objects in the main game
	Most of the objects coordinate are substracted by camera offset position to 
	Draw with respect to the camera position
*/
void Draw_MainGame(){
	//the sky
	iSetColor(11, 0, 36);
	iFilledRectangle(0,0,1280,720); // The ractangle that makes the background blue
	iSetColor(255,255,255);	
	// stars
	for(int i = 0; i<num_of_stars/2; i++){		// The stars are iPoints at previously defined random positions with random radius that make it to blink
		iPoint(stars_pos1[i].x-camera_offset.x*0.7,stars_pos1[i].y-camera_offset.y*0.7,(rand()%100)*.03);
		iPoint(stars_pos2[i].x-camera_offset.x*0.5,stars_pos2[i].y-camera_offset.y*0.5,(rand()%100)*.015);
	}

	// All the active asteroids are drawn here
	for(int i = 0; i<active_ateroids; i++){
		Draw_gameObject(asteroids_properties[i]);	//Draw the poygons using the vertices info 
	}
	if(!gameover)Draw_flare();						// Draw the thrust flare/gas
	Draw_bullet(bullets);							// Draw bullet fired by players
	Draw_bullet(enemy_bullets);						// Draw bullets fired by enemy
	//Drawing each player
	if(player.life>0)Draw_gameObject(player);
	if(multiplayer && player2.life>0)Draw_gameObject(player2);


	for(int i = 0; i<active_enemy; i++){
		Draw_gameObject(enemy_porperties[i]);
	}
	draw_enemy_health_bar();

	draw_explosion();

	if(multiplayer){		// When on multiplayer mode this shows the name of the players
		if(!p1_Dead)iText(player.position.x-camera_offset.x-20,player.position.y+50-camera_offset.y,name_p1,GLUT_BITMAP_8_BY_13);
		if(!p2_Dead)iText(player2.position.x-camera_offset.x-20,player2.position.y+50-camera_offset.y,name_p2,GLUT_BITMAP_8_BY_13);
	}


	#ifdef DEBUG
	char text[50];
	sprintf(text,"%lf %lf %lf",player.position.x,player.position.y,player.angle);
	iSetColor(255,255,255);
	iText(player.position.x-camera_offset.x-50,player.position.y-camera_offset.y-50,text);
	for(int i = 0; i<active_enemy; i++){
		iLine(enemy_porperties[i].position.x-camera_offset.x,enemy_porperties[i].position.y-camera_offset.y,enemy_destination[i].x-camera_offset.x,enemy_destination[i].y-camera_offset.y);
	}
	#endif // DEBUG

	//Game Health bar and texts
	char string[50];
	if(multiplayer){
		sprintf(string,"Wave: %d",wave);
		iText(600,650,string,GLUT_BITMAP_9_BY_15);
		//player2
		iText(1000,670,name_p2,GLUT_BITMAP_TIMES_ROMAN_24);
		sprintf(string, "Health :%g",player2.life);
		iText(1000,630,string,GLUT_BITMAP_8_BY_13);
		iSetColor(2, 145, 227);
		iFilledRectangle(1000,610,220*player2.life/100,8);
		iSetColor(255,255,255);
		iRectangle(1000,610,220,8);
		sprintf(string,"Score: %d",total_score_p2);
		iText(1000,570,string,GLUT_BITMAP_9_BY_15);
		//player1
		iText(50,670,name_p1,GLUT_BITMAP_TIMES_ROMAN_24);
		sprintf(string, "Health :%g",player.life);
		iText(50,630,string,GLUT_BITMAP_8_BY_13);
		iSetColor(2, 145, 227);
		iFilledRectangle(50,610,220*player.life/100,8);
		iSetColor(255,255,255);
		iRectangle(50,610,220,8);
		sprintf(string,"Score: %d",total_score);
		iText(50,570,string,GLUT_BITMAP_9_BY_15);
	}
	else{	// Heath bars and text when on single player mode
		sprintf(string, "Health :%g",player.life);
		iText(1000,670,string,GLUT_BITMAP_8_BY_13);
		iSetColor(2, 145, 227);
		iFilledRectangle(1000,650,220*player.life/100,8);
		iSetColor(255,255,255);
		iRectangle(1000,650,220,8);
		sprintf(string,"Wave: %d",wave);
		iText(600,650,string,GLUT_BITMAP_9_BY_15);
		sprintf(string,"Score: %d",total_score);
		iText(50,650,string,GLUT_BITMAP_9_BY_15);
	}



	// Shows message when a wave is completed
	if(messageTimer>0 && countdown<=0 && !gameover){
		int msg_size = strlen(message[msg]);
		iText(600-msg_size*3,570,message[msg],GLUT_BITMAP_TIMES_ROMAN_24);
	}

	if(countdown>0){
		char count[2];
		iShowImage2(610,500,&count_digit[(int)ceil(countdown)-1],0);
	}

	// All texts showed in game over screen
	if(gameover){
		iShowImage2(400,460,&GameOverText,0);
		char scoretext[50];
		if(!multiplayer){
			sprintf(scoretext,"Total Score: %d",total_score);
			iText(400,300,scoretext,GLUT_BITMAP_9_BY_15);
			sprintf(scoretext,"High Score: %d",high_score);
			iText(680,300,scoretext,GLUT_BITMAP_9_BY_15);
		}
		else{
			iText(400,360,name_p1,GLUT_BITMAP_TIMES_ROMAN_24);
			sprintf(scoretext,"Enemy Killed : %d",kill_count);
			iText(400,300,scoretext,GLUT_BITMAP_9_BY_15);
			sprintf(scoretext,"Asteroids destroyed : %d",destroy_count);
			iText(400,260,scoretext,GLUT_BITMAP_9_BY_15);
			sprintf(scoretext,"Total Score: %d",total_score);
			iText(400,220,scoretext,GLUT_BITMAP_9_BY_15);

			iText(680,360,name_p2,GLUT_BITMAP_TIMES_ROMAN_24);
			sprintf(scoretext,"Enemy Killed : %d",kill_count_p2);
			iText(680,300,scoretext,GLUT_BITMAP_9_BY_15);
			sprintf(scoretext,"Asteroids destroyed : %d",destroy_count_p2);
			iText(680,260,scoretext,GLUT_BITMAP_9_BY_15);
			sprintf(scoretext,"Total Score: %d",total_score_p2);
			iText(680,220,scoretext,GLUT_BITMAP_9_BY_15);
			if(total_score > total_score_p2)iText(500,150,(sprintf(scoretext,"%s  is the winner !",name_p1),scoretext),GLUT_BITMAP_TIMES_ROMAN_24);
			else if(total_score < total_score_p2)iText(500,150,(sprintf(scoretext,"%s  is the winner !",name_p2),scoretext),GLUT_BITMAP_TIMES_ROMAN_24);
			else iText(600,150,"Draw !",GLUT_BITMAP_TIMES_ROMAN_24);
		}
		iText(50,50,"Try Again",GLUT_BITMAP_TIMES_ROMAN_24);
		iText(1080,50,"Main Menu",GLUT_BITMAP_TIMES_ROMAN_24);

		if(ButtonHighlighterpos.y != -1){
			iShowImage2(ButtonHighlighterpos.x,ButtonHighlighterpos.y,&ButtonHighligter,0);
		}
	}
	else if(paused){	//texts showed in paused screen
		iShowImage2(480,460,&PausedText,0);
		iText(400,270,"[Press r to resume]",GLUT_BITMAP_9_BY_15);
		iText(700,270,"[Press e to exit]",GLUT_BITMAP_9_BY_15);
	}
	else{	
		iText(1000,50,"[Press esc to pause the game]",GLUT_BITMAP_9_BY_15);
	}

}


void Draw_mainMenu(){
	iShowImage(0,0,&MainMenuBG);
	for(int i = 0; i<num_of_stars/2; i++){		//Drawing ipoints on predefined random positions as a stars
		iPoint(stars_pos1[i].x-camera_offset.x*0.7,stars_pos1[i].y-camera_offset.y*0.7,(rand()%100)*.03);
		iPoint(stars_pos2[i].x-camera_offset.x*0.5,stars_pos2[i].y-camera_offset.y*0.5,(rand()%100)*.015);
	}
	
	for(int i = 0; i<5; i++){					//showing buttons
		if(i == 1 && !saved_game_available)continue;	// when no saved game, active continue game button is not showd
		iShowImage2((int)buttons[i].x,(int)buttons[i].y,ButtonImage+i,0);
	}
	if(!saved_game_available)iShowImage2((int)buttons[1].x,(int)buttons[1].y,ButtonImage+5,0);	//inactive conitue button is showed when saved game available
	iShowImage2(350,480,&GameLogo,0);
	if(ButtonHighlighterpos.x != -1){		//showing the highlighter when it has a valid position value from ipassivemouse
		iShowImage2(ButtonHighlighterpos.x,ButtonHighlighterpos.y,&ButtonHighligter,0);
	}
}

void Draw_chooseScreen(){
	if(multiplayer){			// when on muliplayer mode
		iShowImage(0,0,&choose_screen_multi);
		iShowImage2(78,500,&TextBox,0x000000);
		iShowImage2(906,500,&TextBox,0x000000);
		iSetColor(255,255,255);
		iText(93,518,name_p1,GLUT_BITMAP_TIMES_ROMAN_24);
		iText(922,518,name_p2,GLUT_BITMAP_TIMES_ROMAN_24);
		
		Draw_gameObject(player);
		Draw_gameObject(player2);
	}else{				// on single player mode
		iShowImage(0,0,&choose_screen_single);
		iSetColor(255,255,255);
		Draw_gameObject(player);
	}
}
// resetting with all the initial values before start
void start(int mode){	
	PlaySound(TEXT("assets/Sound/bgm2.wav"),NULL,SND_LOOP | SND_ASYNC);
	if(mode == multiplayer_mode){		
		player.position = player1_initital_pos;
		player.velocity = {0,0};
		player.scale = 0.7;
		player.life = 100;

		player2.position = player2_inititial_pos;
		player2.velocity = {0,0};
		player2.scale = 0.7;
		player2.life = 100;

		total_score_p2 = 0;
		destroy_count_p2 = 0;
		kill_count_p2 = 0;
		p1_Dead = 0;
		p2_Dead = 0;

		camera_offset.x = (player.position.x+player2.position.x)/2*cam_factor_x-640;
		camera_offset.y = (player.position.y+player2.position.y)/2*cam_factor_y-360;
	}
	else{
		player.position.x = 0;
		player.position.y = 0;
		player.velocity.x = 0;
		player.velocity.y = 0;
		player.scale = 0.7;
		player.life = 100;

		camera_offset.x = player.position.x*cam_factor_x-640;
		camera_offset.y = player.position.y*cam_factor_y-360;
		multiplayer = 0;
		//LOADING HIGHSCORE	
		High_score_Data = fopen("GameData/HighScore.bin","rb");
		fread(&high_score,sizeof(int),1,High_score_Data);
		fclose(High_score_Data);
		saveTime = currTime;
	}

	
	// setting the camera initial position
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
	countdown = 3;
	paused = 0;
	gameover = 0;
	wave = 0;
	total_score = 0;
	destroy_count = 0;
	kill_count = 0;
	msg = 0;
	active_exp_particle = 0;
	messageTimer = 0;
	for(int i = 0; i<max_bullet; i++){
		bullets[i].active = 0;
		enemy_bullets[i].active = 0;
	}

	for(int i = 0; i<total_particle; i++){
		flare[i].life = 0;
	}
	p1_thrust_toggle = p2_thrust_toggle = 0;
	active_enemy = 0;
}

void update_gameplay(){
	currTime = clock();
		
	if(multiplayer){
		if(player.life>0){
			if(p1_thrust_toggle)thrust(&player);
			if(turn_p1>0){							// turn  > 0 rotates right direction smoothly
				player.angle += turn_p1/1*0.07;
				turn_p1 -= dt*10;
				if(turn_p1<0)turn_p1 = 0;
			}
			if(turn_p1<0){							// turn  > 0 rotates left direction smoothly
				player.angle += turn_p1/1*0.07;
				turn_p1 += dt*10;
				if(turn_p1>0)turn_p1 = 0;
			}
		}
		if(player2.life>=0){
			if(p2_thrust_toggle)thrust(&player2);
			if(turn_p2>0){
				player2.angle += turn_p2/1*0.07;
				turn_p2 -= dt*10;
				if(turn_p2<0)turn_p2 = 0;
			}
			if(turn_p2<0){
				player2.angle += turn_p2/1*0.07;
				turn_p2 += dt*10;
				if(turn_p2>0)turn_p2 = 0;
			}
		}
	}
	//updating explosion particles
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

	if(shakeTime<1){			//sinusoidal displacement factor for camera shaking
		shakeTime+=dt*10;
		shake_factor = 20*sin(15*shakeTime)*(1.0-shakeTime);		
	}

	// updating the position of the camera
	camera_offset.x = (multiplayer?(player.position.x+player2.position.x)/2:player.position.x)*cam_factor_x-640-shake_factor;
	camera_offset.y = (multiplayer?(player.position.y+player2.position.y)/2:player.position.y)*cam_factor_y-360-shake_factor;

	if(countdown>0){
		countdown -= dt*5;
		return; //Unill the countdown ends, rest of updates are skiped(paused)
	}

	if(gameover){
		player.life = 0;
		player2.life = 0;
		return;  // When the game is over the rest of the updates of this function is skipped(paused)
	}


	if(!multiplayer&&currTime-saveTime>5000){
		SaveGame();            // After each 5 seconds, All the states of the game are saved
		saveTime = currTime;
	}



	player_movement(&player);
	if(multiplayer)player_movement(&player2);

	//updating flare particles
	int active = 0;
	for(int i = 0; i<total_particle; i++){
		
		if(flare[i].life>0.0000001){
			flare[i].life -= dt;
			flare[i].position += flare[i].velocity*dt;
			active++;
		}
	}
	//updating bullets
	active = 0;
	for(int i = 0; i<max_bullet; i++){
		if(bullets[i].active){				//updating bullets position
			bullets[i].position.x += bullets[i].velocity.x*dt;
			bullets[i].position.y += bullets[i].velocity.y*dt;
			// inactivating bullets  when the are out of the screen
			if(bullets[i].position.x-camera_offset.x<0|| bullets[i].position.x-camera_offset.x>1280 || bullets[i].position.y-camera_offset.y<0||bullets[i].position.y-camera_offset.y>720){
				bullets[i].active = 0;
			}
		}
	}
	//updating enemy bullets
	for(int i = 0; i<max_bullet; i++){
		if(enemy_bullets[i].active){		//updating bullets position
			enemy_bullets[i].position.x += enemy_bullets[i].velocity.x*dt;
			enemy_bullets[i].position.y += enemy_bullets[i].velocity.y*dt;
			// inactivating bullets  when the are out of the screen
			if(enemy_bullets[i].position.x-camera_offset.x<0|| enemy_bullets[i].position.x-camera_offset.x>1280 || enemy_bullets[i].position.y-camera_offset.y<0||enemy_bullets[i].position.y-camera_offset.y>720){
				enemy_bullets[i].active = 0;
			}
		}
	}


	//updating asteroids   -- A minimum count of active asteroids is always maintained. when they are destroyd and less then the lower limit it is generated form the edge of the game world
	while(active_ateroids<asteroid_limit){
		generate_asteroid();
	}
	for(int i = 0; i<active_ateroids; i++){		// destroying astedroids when they are out of the game world
		if(-world_limit_y/2-10> asteroids_properties[i].position.y || asteroids_properties[i].position.y>world_limit_y/2+10 || -world_limit_x/2-10> asteroids_properties[i].position.x || asteroids_properties[i].position.x>world_limit_x/2+10){
			asteroids_properties[i] = asteroids_properties[--active_ateroids];
		}
		asteroids_properties[i].angle += asteroids_properties[i].angular_velocity*dt;			// updating rotation
		if(asteroids_properties[i].angle>=acos(-1)*2)asteroids_properties[i].angle = 0;			
		if(asteroids_properties[i].angle<0)asteroids_properties[i].angle = acos(-1)*2-0.00001;	// keeping the angle within 0 to 2PI
		asteroids_properties[i].position.x+=asteroids_properties[i].velocity.x*dt;				// updating position
		asteroids_properties[i].position.y+=asteroids_properties[i].velocity.y*dt;
		
	}


	if(player.life>0)player_collision(&player);					//handling all the collisions of player 1
	if(multiplayer && player2.life>0)player_collision(&player2);//handiling all the collisions of player 2

	// asteroids+enemy bullet collision
	for(int i = 0; i<20; i++){
		if(!bullets[i].active)continue;
		for(int j = 0; j<active_ateroids; j++){	//checking all active bullets and asteroids if they are overlaping	
			if(distance(asteroids_properties[j].position,bullets[i].position)<asteroids_properties[j].object.collider_radius*asteroids_properties[j].scale)
			{	//creating smaller asteroids
				if(asteroids_properties[j].scale == asteroid_scale1){
					create_asteroids(asteroids_properties[j].position,asteroid_scale2);
					create_asteroids(asteroids_properties[j].position,asteroid_scale2);
				}
				else if(asteroids_properties[j].scale == asteroid_scale2){
					create_asteroids(asteroids_properties[j].position,asteroid_scale3);
					create_asteroids(asteroids_properties[j].position,asteroid_scale3);
					create_asteroids(asteroids_properties[j].position,asteroid_scale3);
				}
				//Destroying larger asteroids
				Destroy_asteroid(j);
				explode(bullets[i].position,{0,0},1);
				bullets[i].active = 0;
				if(bullets[i].player == 0) destroy_count++;
				else destroy_count_p2++;
				
			}
		}
		for(int j = 0; j<active_enemy; j++){	//checking collisoin with all the active enemies and bullets
			if(distance(enemy_porperties[j].position,bullets[i].position)<enemy_porperties[j].object.collider_radius*enemy_porperties[j].scale)
			{
				enemy_porperties[j].life -= 20;
				bullets[j].active = 0;
				explode(bullets[i].position,{0,0},1);
				if(bullets[i].player == 0)total_score+=50;
				else total_score_p2+=50;
			}
			if(enemy_porperties[j].life <= 0){	//enemy killed
				destroy_enemy(j);
				if(bullets[i].player == 0) kill_count++;
				else kill_count_p2++;
			}
		}
	}
	//updating score
	total_score = kill_count*200+destroy_count*150;
	total_score_p2 = kill_count_p2*200+destroy_count_p2*150;
	ControlEnemy();
	enemy_attack();
	//when enemy count is 0. a message is sent and wave is increased
	if(active_enemy == 0){
		if(wave>0)messageTimer = 1;
		msg = rand()%12;
		SendEnemy(++wave);
	}
	if(messageTimer>0)messageTimer -= dt;
	// handling deaths of players
	if(multiplayer){
		if(player.life<=0 && !p1_Dead){
			explode(player.position,player.velocity,0);
			player.life = 0;
			p1_Dead = 1;
		}
		if(player2.life<=0 && !p2_Dead){
			explode(player2.position,player2.velocity,0);
			player2.life = 0;
			p2_Dead = 1;
			p2_thrust_toggle = 0;
		}
		if(player.life<=0 && player2.life <= 0){	//gameover when both are dead
			gameover = 1;
			iResizeImage(&ButtonHighligter,140,45);
			paused = 1;
			p1_thrust_toggle = 0;
		}
	}
	else{
		if(player.life<=0){
			gameover = 1;
			iResizeImage(&ButtonHighligter,140,45);
			explode(player.position,player.velocity,0);
			paused = 1;
			player.life = -0.001;
			if(total_score>high_score){
				high_score = total_score;
				High_score_Data = fopen("GameData/highscore.bin","wb");
				fwrite(&high_score,sizeof(int),1,High_score_Data);
				fclose(High_score_Data);
			}
		}

	}
	//players inter shooting time delay
	if(reloadTime_p1>0){	
		reloadTime_p1-=dt;
	}
	if(reloadTime_p2>0){
		reloadTime_p2-=dt;
	}

}
// this function activates thrust gass particles and give force toward the direction of the spaceship
void thrust(object_properties *player){
	double velocity_magnitude = sqrt(player->velocity.x*player->velocity.x+player->velocity.y*player->velocity.y);
	if(velocity_magnitude<max_velocity){	// adding velocity upto max limit
		player->velocity.x = player->velocity.x + acceleration*cos(player->angle)*dt;
		player->velocity.y = player->velocity.y + acceleration*sin(player->angle)*dt;
	}
	
	int cnt = 0;
	for(int i = 0; i<total_particle; i++){		//setting up gas partivle velocity and position
		if(flare[i].life<0.00000001){
			cnt++;
			flare[i].life = particle_life;
			flare[i].velocity = rand()%1000;
			flare[i].position = particle_initial_pos*player->scale;
			flare[i].spaceship = player;
		}       

		if(cnt>flare_intensity)break;
	}
}

// when called the player of the parameter fires a bullets
//setting angle and velocity of the bullets
void fire(object_properties *player){
	for(int i = 0; i<max_bullet; i++){
		if(!bullets[i].active){
			bullets[i].position.x = player->position.x+60*cos(player->angle);
			bullets[i].position.y = player->position.y+60*sin(player->angle);
			bullets[i].velocity.x = bullet_velocity*cos(player->angle);
			bullets[i].velocity.y = bullet_velocity*sin(player->angle);
			bullets[i].active = 1;
			bullets[i].player = (player == &player2); //who fired it
			break;
		}
	}
	if(player == &player2)reloadTime_p2 = player_rt;
	else reloadTime_p1 = player_rt;
	playsound("assets/Sound/fire.wav");
}

//Generates asteroids automatically on the edge of the game world when active asteroid count is less than max count
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
// checking if two object is colliding or not
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

// The AI control of the enemy
// This function sets a target location in the screen of the enemy to where it moves toward, when it reaches, another new target loation is set
void ControlEnemy(){
	for(int i = 0; i<active_enemy; i++){
		if(distance(enemy_destination[i],enemy_porperties[i].position)<300){	//when destination reached new destination is set
			enemy_destination[i].x = camera_offset.x+rand()%1280;
			enemy_destination[i].y = camera_offset.y+rand()%720;
		}
		// controlling direction.. Smoothly rotates to the target angle
		enemy_direction[i] = atan2(-enemy_porperties[i].position.y+enemy_destination[i].y,-enemy_porperties[i].position.x+enemy_destination[i].x);
		if(enemy_direction[i]<0){
			enemy_direction[i]+=acos(-1)*2;
		}
		if(abs(enemy_direction[i]-enemy_porperties[i].angle)<0.05){
			enemy_porperties[i].angle = enemy_direction[i];
		}
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
		// updating position according to velocity and direction
		enemy_porperties[i].velocity.x = max_velocity*cos(enemy_porperties[i].angle);
		enemy_porperties[i].velocity.y = max_velocity*sin(enemy_porperties[i].angle);
		enemy_porperties[i].position.x += enemy_porperties[i].velocity.x*dt;
		enemy_porperties[i].position.y += enemy_porperties[i].velocity.y*dt;
	}
	
}
void SendEnemy(int wave){	// when this function is called wave numbers of enemy are spawned.
	active_enemy = wave;
	for(int i = 0; i<active_enemy; i++){
		enemy_porperties[i].object = enemy[rand()%3];
		enemy_porperties[i].life = 100;
		enemy_porperties[i].scale = 0.4;
		enum side x = static_cast<side>(rand()%4);	// choosing a random edge
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
		enemy_destination[i] = enemy_porperties[i].position;	//initial destination is set to its current postion so that new postion is set intantly
		enemy_reload_time[i] = rand()%(int)reload_time;	
	}
}
void destroy_enemy(int index){	// enemy is destroyed by replacing the destroyed enemy's index with the last index's information and the enemy count is decreased
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
void enemy_attack(){	//fires a bullet toward the nearest player when called
	for(int i = 0; i<active_enemy; i++){
		if(distance(enemy_porperties[i].position,camera_offset)>1000)continue;	// when out of the screen firing is skipped
		if(enemy_reload_time[i]>0.00001){
			enemy_reload_time[i] -= 5;
			continue;
		}

		double enemy_player_angle = atan2(player.position.y-enemy_porperties[i].position.y,player.position.x-enemy_porperties[i].position.x);

		if(p1_Dead || (!p2_Dead && multiplayer && distance(enemy_porperties[i].position,player.position)>distance(enemy_porperties[i].position,player2.position))){
			enemy_player_angle = atan2(player2.position.y-enemy_porperties[i].position.y,player2.position.x-enemy_porperties[i].position.x);
		}

		for(int j = 0; j<max_bullet; j++){
			if(enemy_bullets[j].active)continue;
			enemy_bullets[j].position = enemy_porperties[i].position;
			enemy_bullets[j].velocity.x = bullet_velocity*cos(enemy_player_angle);
			enemy_bullets[j].velocity.y = bullet_velocity*sin(enemy_player_angle);
			enemy_bullets[j].active = 1;
			enemy_reload_time[i] = reload_time;
			playsound("assets/Sound/fire.wav");
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

// activating explosion particles 
void explode(vector2 pos,vector2 velocity,bool collision){
	int particle_count = 100;			// total number of particles during each explosion
	if(collision)particle_count = 10;	// totol number of particle in case of bullet's collision
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
			if(i<60 ){
				v = rand()%50;
				explosion[active_exp_particle].radius = 10;	// the central circular cloud
			}
			else{
				v = 500+rand()%500;
				explosion[active_exp_particle].radius = 1.5;	// for the high speed spreading particle
			}
		}
		explosion[active_exp_particle].angle = angle*acos(-1)/180;
		explosion[active_exp_particle].velocity.x += v*cos(angle);
		explosion[active_exp_particle].velocity.y += v*sin(angle);
		explosion[active_exp_particle].life = 0.2;
		active_exp_particle++;
	}
		
	if(!collision){	//playing explosion sound
		startTime_explosion = currTime;
		playsound("assets/Sound/explode.wav");
		shakeTime = 0;
	}
	else{
		playsound("assets/Sound/exp2.wav");
	}
}

void draw_explosion(){
	iSetColor(255,255,255);
	for(int i = 0; i<active_exp_particle; i++){
		iFilledCircle(explosion[i].position.x-camera_offset.x,explosion[i].position.y-camera_offset.y,(explosion[i].radius)*(explosion[i].life/0.2));
	}
}
// Loading all the assets from the files at the start of the game
void Load_resources(){
	initializeGameobject(&asteroids[0],"assets/GameObjects/asteroid1.txt");
	initializeGameobject(&asteroids[1],"assets/GameObjects/asteroid2.txt");
	initializeGameobject(&asteroids[2],"assets/GameObjects/asteroid3.txt");
	initializeGameobject(&bullet_obj,"assets/GameObjects/bullet.txt");


//Testing enemy sprite
	initializeGameobject(&enemy[0],"assets/GameObjects/Enemy1.txt");
	initializeGameobject(&enemy[1],"assets/GameObjects/Enemy2.txt");
	initializeGameobject(&enemy[2],"assets/GameObjects/Enemy3.txt");

	initializeGameobject(&spaceship[0],"assets/GameObjects/spaceship.txt");
	initializeGameobject(&spaceship[1],"assets/GameObjects/spaceship2.txt");
	initializeGameobject(&spaceship[2],"assets/GameObjects/spaceship3.txt");

	iLoadImage(&MainMenuBG,"assets/Background/MainMenu_bg.png");
	iResizeImage(&MainMenuBG,1280,720);
	iLoadImage(&GameLogo,"assets/png/AstroStrike.png");
	iResizeImage(&GameLogo,580,180);
	iLoadImage(&ButtonImage[0],"assets/Buttons/StartGame.png");
	iLoadImage(&ButtonImage[1],"assets/Buttons/ContinueGame.png");
	iLoadImage(&ButtonImage[2],"assets/Buttons/Multiplayer.png");
	iLoadImage(&ButtonImage[3],"assets/Buttons/About.png");
	iLoadImage(&ButtonImage[4],"assets/Buttons/Exit.png");
	iLoadImage(&ButtonImage[5],"assets/Buttons/x.png");
	iLoadImage(&ButtonHighligter,"assets/Buttons/HoverBar.png");
	iLoadImage(&GameOverText,"assets/png/GameOverText.png");
	iLoadImage(&PausedText,"assets/png/PausedText.png");
	iLoadImage(&count_digit[0],"assets/png/1.png");
	iLoadImage(&count_digit[1],"assets/png/2.png");
	iLoadImage(&count_digit[2],"assets/png/3.png");
	iLoadImage(&TextBox,"assets/png/TextBox.png");
	iLoadImage(&choose_screen_multi,"assets/Background/ChooseNameMultiplayer.png");
	iLoadImage(&choose_screen_single,"assets/Background/ChooseSpaceshipScreen.png");
	iLoadImage(&aboutPage,"assets/Background/AboutPage.png");
	

	//setting random postion of the stars
	for(int i = 0; i<num_of_stars/2; i++){
		stars_pos1[i].x = -world_limit_x/2+rand()%(int)world_limit_x;
		stars_pos1[i].y = -world_limit_y/2+rand()%(int)world_limit_y;
		stars_pos2[i].x = -world_limit_x/2+rand()%(int)world_limit_x;
		stars_pos2[i].y = -world_limit_y/2+rand()%(int)world_limit_y;
	}
	saved_data = fopen("GameData/gamedata.bin","rb");
	saved_game_available = 0;
	if(is_file_empty(saved_data) ){
		saved_game_available = 0;
	}else{
		if(fread(&saved_game_available,sizeof(bool),1,saved_data)!=1){
			printf("Error in reading first bool\n");
		}
	}

}

void SaveGame(){
	saved_data = fopen("GameData/gamedata.bin","wb");
	if(!saved_data){
		perror("Game Data openning failed!");
		exit(EXIT_FAILURE);
	}
	saved_game_available = 1;
	fwrite(&saved_game_available,sizeof(bool),1,saved_data);
	fwrite(&player,sizeof(object_properties),1,saved_data);
	fwrite(&wave,sizeof(object_properties),1,saved_data);
	fwrite(&total_score,sizeof(int),1,saved_data);
	fwrite(&destroy_count,sizeof(int),1,saved_data);
	fwrite(&kill_count,sizeof(int),1,saved_data);
	fwrite(&active_enemy,sizeof(int),1,saved_data);
	//enemy
	fwrite(enemy_porperties,sizeof(object_properties),max_enemy,saved_data);
	fwrite(enemy_destination,sizeof(vector2),max_enemy,saved_data);
	fwrite(enemy_direction,sizeof(double),max_enemy,saved_data);
	fwrite(enemy_reload_time,sizeof(double),max_enemy,saved_data);
	//particle
	fwrite(flare,sizeof(thrust_particle),total_particle,saved_data);
	fwrite(explosion,sizeof(explosion_particle),total_exp_particle,saved_data);
	fwrite(&active_exp_particle,sizeof(int),1,saved_data);
	//bullets
	fwrite(bullets,sizeof(bullet),max_bullet,saved_data);
	fwrite(&bullet_velocity,sizeof(double),1,saved_data);
	fwrite(enemy_bullets,sizeof(bullet),max_bullet,saved_data);
	//asteroids
	fwrite(asteroids_properties,sizeof(object_properties),max_asteroids,saved_data);
	fwrite(&active_ateroids,sizeof(int),1,saved_data);
	fwrite(&asteroid_limit,sizeof(int),1,saved_data);
	//environment
	fwrite(stars_pos1,sizeof(vector2),num_of_stars/2,saved_data);
	fwrite(stars_pos2,sizeof(vector2),num_of_stars/2,saved_data);
	fwrite(&camera_offset,sizeof(vector2),1,saved_data);
	fwrite(&spaceship_in1,sizeof(int),1,saved_data);
	fclose(saved_data);

}
void LoadGame(){
	if(!saved_data){
		perror("Game Data openning failed!");
		exit(EXIT_FAILURE);
	}
	
	fread(&player,sizeof(object_properties),1,saved_data);
	fread(&wave,sizeof(object_properties),1,saved_data);
	fread(&total_score,sizeof(int),1,saved_data);
	fread(&destroy_count,sizeof(int),1,saved_data);
	fread(&kill_count,sizeof(int),1,saved_data);
	fread(&active_enemy,sizeof(int),1,saved_data);
	//enemy
	fread(enemy_porperties,sizeof(object_properties),max_enemy,saved_data);
	fread(enemy_destination,sizeof(vector2),max_enemy,saved_data);
	fread(enemy_direction,sizeof(double),max_enemy,saved_data);
	fread(enemy_reload_time,sizeof(double),max_enemy,saved_data);
	//particle
	fread(flare,sizeof(thrust_particle),total_particle,saved_data);
	fread(explosion,sizeof(explosion_particle),total_exp_particle,saved_data);
	fread(&active_exp_particle,sizeof(int),1,saved_data);
	//bullets
	fread(bullets,sizeof(bullet),max_bullet,saved_data);
	fread(&bullet_velocity,sizeof(double),1,saved_data);
	fread(enemy_bullets,sizeof(bullet),max_bullet,saved_data);
	//asteroids
	fread(asteroids_properties,sizeof(object_properties),max_asteroids,saved_data);
	fread(&active_ateroids,sizeof(int),1,saved_data);
	fread(&asteroid_limit,sizeof(int),1,saved_data);
	//environment
	fread(stars_pos1,sizeof(vector2),num_of_stars/2,saved_data);
	fread(stars_pos2,sizeof(vector2),num_of_stars/2,saved_data);
	fread(&camera_offset,sizeof(vector2),1,saved_data);
	fread(&spaceship_in1,sizeof(int),1,saved_data);



	player.object = spaceship[spaceship_in1];
	for(int i = 0; i<active_enemy; i++){
		enemy_porperties[i].object = enemy[rand()%3];
	}
	for(int i = 0; i<active_ateroids; i++){
		asteroids_properties[i].object = asteroids[rand()%3];
	}
	fclose(saved_data);

	PlaySound(TEXT("assets/Sound/bgm2.wav"),NULL,SND_LOOP | SND_ASYNC);

}
int is_file_empty(FILE *file) {
    fseek(file, 0, SEEK_END); 
    long size = ftell(file); // Get the position in byte
    rewind(file);
    return size == 0; 
}
void player_movement(object_properties *player){
	//updating the position of the player
	player->position.x += player->velocity.x*dt;
	player->position.y += player->velocity.y*dt;


	// limiting spaceship movement within world limit
	if(player->position.x-camera_offset.x<0){
		player->position.x = camera_offset.x+20;
		player->velocity.x += 500;
	}
	if(player->position.x-camera_offset.x>1280){
		player->position.x = camera_offset.x+1275;
		player->velocity.x -= 500;
	}
	if(player->position.y-camera_offset.y<0){
		player->position.y = camera_offset.y+10;
		player->velocity.y += 500;
	}
	if(player->position.y-camera_offset.y>720){
		player->position.y = camera_offset.y+715;
		player->velocity.y -= 500;
	}

	//adding friction
	double velocity_magnitude = sqrt(player->velocity.x*player->velocity.x+player->velocity.y*player->velocity.y);
	if(abs(player->velocity.x)>0){
		player->velocity.x -= player->velocity.x/velocity_magnitude*friction*dt;
	}
	if(abs(player->velocity.y)>0){
		player->velocity.y -= player->velocity.y/velocity_magnitude*friction*dt;
	}

}

void player_collision(object_properties *player){
	// asteroid player collision
	for(int i = 0; i<active_ateroids; i++){
		if(isColliding(*player,asteroids_properties[i])){
			player->life -= 10*asteroids_properties[i].scale;
			explode(asteroids_properties[i].position,{0,0},0);
			Destroy_asteroid(i);
			
		}
	}

	// player enemy collision
	for(int i = 0; i<active_enemy; i++){
		if(isColliding(*player,enemy_porperties[i])){
			if(actv_pl_en_col){
				player->life -= 15;
				actv_pl_en_col = 0;
			}
			destroy_enemy(i);
		}
		else actv_pl_en_col = 1;
	}

	//player enemy_bullet collision
	for(int i = 0; i < max_bullet; i++){
		if(!enemy_bullets[i].active)continue;
		if(distance(player->position,enemy_bullets[i].position)<player->object.collider_radius){
			player->life -= 5;
			explode(enemy_bullets[i].position,{0,0},1);
			enemy_bullets[i].active = 0;
		}
	}
}

void playsound(char filename[])
{
	char command[256];
	sprintf(command, "close sound%d", soundcount);
    mciSendString(command, NULL, 0, NULL);
    sprintf(command, "open \"%s\" type waveaudio alias sound%d",filename, soundcount);
    mciSendString(command, NULL, 0, NULL);
	sprintf(command, "play sound%d", soundcount);
    mciSendString(command, NULL, 0, NULL);
    soundcount = (soundcount+1)%max_sound;
}




// Shahtaz Rahman Labib 