
//IMPORTAINT: Just go to line 103, the rest isnt your concern; code bricks without printf in file_size(), dunno why
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define max(a, b) (a > b ? a : b)

//---------------------------bicagis-code-START-----------------------------//

struct pos
{
	int x, y;
};

int file_size(FILE *file) // in bytes
{
	fseek(file, 0, SEEK_END); // sets file position to EOF
	int rval = (int)ftell(file); // get file size; we assume the file is smaller than INT_MAX
	rewind(file); // set file pos to start
	printf("\n"); //                    FOR SOME REASON THIS PREVENTS A MALLOC ERROR. HOW??? DONT REMOVE IT // honestly I'm amazed how you broke it so badly that it needs this
	return rval; 
}

char* file_to_buffer(FILE *file, int *out_width, int *out_height) // must be a rectangle, else it doesnt work
{
	char *buffer = malloc(file_size(file));

	fgets(buffer, INT_MAX, file); // get first row
	*out_width = strlen(buffer);
	*out_height = file_size(file) / *out_width;

	for(int i = 0; i < *out_height; i++) // get all the rows
	{
		fgets(buffer + i * *out_width, INT_MAX, file);
	}
	return buffer;
}

struct pos *buffer_to_array(char* buffer, int width, int height, int *out_n_nodes) // array of # positions, terminates with x,y = INT_MAX; out_n_nodes - number of #'s
{
	int n_nodes = 0;
	struct pos *array = malloc(sizeof(struct pos) * width * height);
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			if(buffer[i*width + j]  == '#')
			{
				array[n_nodes].x = j;
				array[n_nodes].y = i;
				n_nodes++;
			}
		}
	}
	array[n_nodes].x = INT_MAX; // terminates the array
	array[n_nodes].y = INT_MAX;
	*out_n_nodes = n_nodes;
	return array;
}

// matrix[y * n_nodes + x] is element (x,y)
int *array_to_matrix(struct pos *array, int n_nodes, int (*dist)(struct pos, struct pos), struct pos start_pos, int *start_vertex) // could be short to save mem but nah; takes a distance function, self explanatory
{
	int *matrix = malloc(n_nodes * n_nodes);
	for(int i = 0; i < n_nodes; i++)
	{
		if (array[i].x == start_pos.x && array[i].y == start_pos.y)
			*start_vertex = i;
		for(int j = 0; j < n_nodes; j++)
		{
			matrix[i * n_nodes + j] = (*dist)(array[i], array[j]); // takes the dist of the ith and the jth elements and writes it in matrix[i][j]
		}
	}
	return matrix;
}

//---------------------------bicagis-code-END-----------------------------//

int func(struct pos a, struct pos b)
{
	double x = abs(b.x - a.x) - 1, y = b.y - a.y; // calculate the distance Pesho needs to jump
	if (x == 0) {
		if (y <= 0) // if it's next to or below
			return 0;
		x += 0.0625; // this adds just a bit of differance so that the formula works when x is 0
		// it shouldn't be a problem, but if it is just make it even lower (not 0 of course) 
	}
	const double gravity = 9.80665; // gravity acceleration assuming it's on earth and one grid metric is 1 meter
	double offset = (M_PI / 2 - atan(y / x)) / 2; // half of the straight angle to the platform
	double angle = M_PI / 2 - offset, angleUp, angleDown; // angle of the initial jump
	double gxx2 = gravity * x * x / 2; // going to need this every time I calculate velocity
	double vmin = sqrt(gxx2 / (x * tan(angle) - y)) / cos(angle), velocityMid, velocityUp, velocityDown; // velocity
	velocityMid = vmin;
	// this is a binary search for the minimum velocity
	// first it calculates the velocity for the upper and lower angle
	// it compares them and chooses the smaller one
	// if the smaller one is less than the current smallest it becomes the current smallest
	for (int i = 0; i < 10; i++) { // currently the resolution is 1/1024 of a degree
	// it's determined by how long it iterates so just change the 10 to whatever you want (don't forget it needs to fit in a double)
		offset /= 2;
		angleUp = angle + offset;
		angleDown = angle - offset;
		velocityUp = sqrt(gxx2 / (x * tan(angleUp) - y)) / cos (angleUp); // this is the formula for calculating the velocity as you saw 10 lines above
		velocityDown = sqrt(gxx2 / (x * tan(angleDown) - y)) / cos (angleDown); // took me 2 hours and desmos accoount to figure it out
		// this formula is good and all but it's going to be a nightmare to add air resistance
		if (velocityUp > velocityDown) {
			velocityMid = velocityDown;
			angle = angleDown;
		} else {
			velocityMid = velocityUp;
			angle = angleUp;
		}
		if (vmin > velocityMid)
			vmin = velocityMid;
	}
	//     vvvv - rounding up because I'm returning an integer
	return ceil(pow(vmin / 3.13156, 2)); // 3.13156 is a constant for converting velocity into horizontal distance with gravity acceleration 9.80665
	// It's solely dependant on the gravity acceleration but I don't know how to calculate it
	// also for gravity acceleration 9.8 (rounded) it's 3.1305, cheers
}

#define DIST func

void bfs(int *matrix, int n_nodes, int start_vertex, int *requiredToReach) 
{
	for (int i = 0; i < n_nodes; i++) {
		if (i == start_vertex)
			continue;
		
		if (requiredToReach[i] > max(requiredToReach[start_vertex], matrix[start_vertex * n_nodes + i])) {
			requiredToReach[i] = max(requiredToReach[start_vertex], matrix[start_vertex * n_nodes + i]);
			bfs(matrix, n_nodes, i, requiredToReach);
		}
	}
}

int pathFinder(char *map, int width, int height, int start_vertex, int destination, int *matrix, int n_nodes, struct pos *array, int *visited, int pesho)
{
	if (start_vertex == destination)
	{
		map[array[start_vertex].y * width + array[start_vertex].x] = 'X';
		return 1;
	}
	for (int i = 0; i < n_nodes; i++) {
		if (visited[i] == 0 && matrix[start_vertex * n_nodes + i] <= pesho) {
			visited[i] = 1;
			if (pathFinder(map, width, height, i, destination, matrix, n_nodes, array, visited, pesho)) {
				map[array[start_vertex].y * width + array[start_vertex].x] = 'X';
				return 1;
			}
		}
	}
	return 0;		
}

void main() // also writtern by bicagis // you wish
{
	char ipath[256];
	printf("Enter path to map.txt: ");
	scanf("%s", ipath);
	FILE *map = fopen(ipath, "r");
	int width, height, fsize = file_size(map); //remove fsize, its just for the printf bellow n nothing else
	char *buffer = file_to_buffer(map, &width, &height);
	fclose(map);
	int n_nodes;
	struct pos *array = buffer_to_array(buffer, width, height, &n_nodes);
//	free(buffer); // NOOO, don't take the buffer away from me, I need it
	int start_vertex;
	struct pos start_pos = {0, 0}; // added these so that I know where the start is
	scanf("%d %d", &start_pos.x, &start_pos.y);
	int *matrix = array_to_matrix(array, n_nodes, &DIST, start_pos, &start_vertex); // DIST - your distance function of choice, placeholder code
//	free(array); // NOOO, don't take the array away from me, I need it

	//printf("file size: %d\n", fsize);
	//printf("width, height: %d,%d\n", width, height);
	//printf("number of #'s: %d\n", n_nodes);
	//if(n_nodes > 1)
	//	printf("dist from 1st to 2nd #: %d\n", matrix[1 * n_nodes + 2]);

	// only thing that you care about is matrix and n_nodes beyond this point, that and DIST, also array_to_matrix() comment
	int *requiredToReach = malloc(n_nodes * sizeof(int)), pesho, policeman, max = 0, n_policemen;
	for (int i = 0; i < n_nodes; i++) {
		requiredToReach[i] = INT_MAX;
	}
	requiredToReach[start_vertex] = 0;
	scanf("%d", &pesho);
	scanf("%d", &n_policemen);
	for (int i = 0; i < n_policemen; i++) {
		scanf("%d", &policeman);
		if (policeman > max)
			max = policeman;
	}
	bfs(matrix, n_nodes, start_vertex, requiredToReach);
	if (max >= pesho)
	{
		printf("IMPOSSIBLE");
		return;
	}
	int destination = -1;
	for (int i = 0; i < n_nodes; i++) {
		if (requiredToReach[i] <= pesho && requiredToReach[i] > max) {
			destination = i;
			break;
		}
	}
	if (destination == -1) {
		printf("IMPOSSIBLE");
		return;
	}
	int *visited = malloc(n_nodes * sizeof(int));
	for (int i = 0; i < n_nodes; i++) {
		visited[i] = 0;
	}
	visited[start_vertex] = 1;
	pathFinder(buffer, width, height, start_vertex, destination, matrix, n_nodes, array, visited, pesho);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++)
			printf("%c", buffer[i * width + j]);
	}
}
