// #include<stdio.h>

struct vector2
{
	double x;
	double y;
};

struct gameObject{
	int number_of_polygons;
	double **x;  // x coordinate of all polygons 
	double **y;  // y coordinate of all polygons
	int *size; // number of nodes of each polygons
	int (*color)[3];
};

struct object_properties{
	gameObject object;
	vector2 position;
	vector2 velocity;
	double angle;
};

struct particle{
	vector2 position;
	vector2 velocity;
	double life = 0;
	
};
