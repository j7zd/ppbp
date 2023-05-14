
//IMPORTAINT: Just go to line 103, the rest isnt your concern; code bricks without printf in file_size(), dunno why

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

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
	return 1;
}

#define DIST func

void main() // also writtern by bicagis
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
