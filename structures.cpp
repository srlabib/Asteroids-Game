
struct vector2
{
	double x;
	double y;
};
/* 	GameObject stores all the verteces of each polygon of the objects
	that is fetched from the text files and the data are stored in the
	memory by dynamically allocating space according to number of vertices

	vertices are w.r.t the center of the object 
*/
struct gameObject{
	int number_of_polygons;
	double **x;                  // x coordinate of all polygons  
	double **y;                  // y coordinate of all polygons
	int *size;                   // number of nodes of each polygons
	int (*color)[3];             // RGB color code
	double collider_radius;
};
// Object properties holds all the information of the objects
struct object_properties{
	gameObject object;
	vector2 position;
	vector2 velocity;
	double angular_velocity;
	double scale;
	double angle;
	double life = 0;
};
// This holds the info of each particle(tiny iCircles) those are emmited as thrust gas
struct thrust_particle{
	object_properties *spaceship; // the spaceship to which the particle corresponds to (player1/player2)
	double position;
	double velocity;
	double life = 0;
};
//This holds the info of each particle that are used as explosion effect
struct explosion_particle{
	vector2 position;
	vector2 velocity;
	double life;
	double angle;
	double radius; 
};
// info of each bullets
struct bullet{
	vector2 position;
	vector2 velocity;
	bool active = 0;
	bool player;   //who fired the bullet.. player1/player2 in multiplayer
};


enum side{
	bottom,top,left,right
};

enum page{
	MainMenu,HighScore,EnterName,GameOver,AboutPage,Game,multiplayer_mode,choose
};

enum button {
	startgame,continuegame,Multi_player,About,exitt
};

char message[13][20] = {
	"Nice Work!",
	"Well Done!",
	"You're on fire!",
	"Impressive!",
	"Keep it up!",
	"Awesome!",
	"You're unstoppable!",
	"Fantastic!",
	"Wave conquered!",
	"Outstanding!",
	"You're crushing it!",
	"Legendary!",
	"Victory secured!"
};
