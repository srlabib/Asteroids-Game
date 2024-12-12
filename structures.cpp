
struct vector2
{
	double x;
	double y;
};

struct gameObject{
	int number_of_polygons;
	double **x;                  // x coordinate of all polygons  
	double **y;                  // y coordinate of all polygons
	int *size;                   // number of nodes of each polygons
	int (*color)[3];             // RGB color code
	double collider_radius;
};

struct object_properties{
	gameObject object;
	vector2 position;
	vector2 velocity;
	double angular_velocity;
	double scale;
	double angle;
	double life = 0;
};

struct thrust_particle{
	object_properties *spaceship;
	double position;
	double velocity;
	double life = 0;
};

struct explosion_particle{
	vector2 position;
	vector2 velocity;
	double life;
	double angle;
	double radius; 
};

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
	MainMenu,HighScore,EnterName,GameOver,credit,Game,multiplayer_mode,choose
};

enum button {
	startgame,continuegame,Multi_player,exitt
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
