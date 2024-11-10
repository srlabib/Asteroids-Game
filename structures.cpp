
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
	int (*color)[3];             //RGB color code
};

struct object_properties{
	gameObject object;
	vector2 position;
	vector2 velocity;
	double angle;
};

struct particle{
	double position;
	double velocity;
	double life = 0;
};

struct bullet{
	vector2 position;
	vector2 velocity;
	bool active = 0;
};
