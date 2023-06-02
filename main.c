
//IMPORTAINT: Just go to line 103, the rest isnt your concern; code bricks without printf in file_size(), dunno why

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <float.h>

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
	printf("\n"); //                    FOR SOME REASON THIS PREVENTS A MALLOC ERROR. HOW??? DONT REMOVE IT
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
int *array_to_matrix(struct pos *array, int n_nodes, int (*dist)(struct pos, struct pos)) // could be short to save mem but nah; takes a distance function, self explanatory
{
	int *matrix = malloc(n_nodes * n_nodes);
	for(int i = 0; i < n_nodes; i++)
	{
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
		x += 0.0625; // this adds just a bit of differance so that the formula works
	}
	const double gravity = 9.80665; // gravity acceleration assuming it's on earth and one grid metric is 1 meter
	double offset = (M_PI / 2 - atan(y / x)) / 2; // half of the straight angle to the platform
	double angle = M_PI / 2 - offset, angleUp, angleDown; // angle of the initial jump
	double gxx2 = gravity * x * x / 2; // going to need this every time I calculate velocity
	double vmin = sqrt(gxx2 / (x * tan(angle) - y)) / cos(angle), velocityMid, velocityUp, velocityDown; // velocity
	velocityMid = vmin;
	for (int i = 0; i < 10; i++) {
		offset /= 2;
		angleUp = angle + offset;
		angleDown = angle - offset;
		velocityUp = sqrt(gxx2 / (x * tan(angleUp) - y)) / cos (angleUp);
		velocityDown = sqrt(gxx2 / (x * tan(angleDown) - y)) / cos (angleDown);
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
	return pow(vmin / 3.1305, 2); // 3.1305 is a constant for converting velocity into distance
}

#define DIST func

void main() // also writtern by bicagis // you wish
{
	FILE *map = fopen("map.txt", "r");
	int width, height, fsize = file_size(map); //remove fsize, its just for the printf bellow n nothing else
	char *buffer = file_to_buffer(map, &width, &height);
	fclose(map);
	int n_nodes;
	struct pos *array = buffer_to_array(buffer, width, height, &n_nodes);
	free(buffer);
	int *matrix = array_to_matrix(array, n_nodes, &DIST); // DIST - your distance function of choice, placeholder code
	free(array);

	//printf("file size: %d\n", fsize);
	printf("width, height: %d,%d\n", width, height);
	printf("number of #'s: %d\n", n_nodes);
	if(n_nodes > 1)
		printf("dist from 1st to 2nd #: %d\n", matrix[1 * n_nodes + 2]);

	// only thing that you care about is matrix and n_nodes beyond this point, that and DIST, also array_to_matrix() comment

}
