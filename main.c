//IMPORTAINT: Just go to line 103, the rest isnt your concern; code bricks without printf in file_size(), dunno why // UPDATE: Fixed
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

//---------------------------bicagis-code-START-----------------------------//

struct pos
{
	int x, y;
};

int file_size(FILE *file) // in bytes
{
	fseek(file, 0, SEEK_END); 	// sets file position to EOF
	int rval = (int)ftell(file); 	// get file size
	rewind(file); 			// set file pos to start
	return rval;
}

// file must be a rectangle, else it doesnt work
// out_width/height - where to write file width/height
char* file_to_buffer(FILE *file, int *out_width, int *out_height)
{
	char *buffer = malloc(file_size(file) + 1);

	fgets(buffer, INT_MAX, file); // get first row
	*out_width = strlen(buffer);
	*out_height = file_size(file) / *out_width;

	for(int row = 0; row < *out_height; row++) // get all the rows
	{
		fgets(buffer + row * *out_width, INT_MAX, file);
	}
	return buffer;
}

// array of # positions
// out_n_nodes - number of #'s
struct pos *buffer_to_array(char* buffer, int width, int height, int *out_n_nodes)
{
	int n_nodes = 0;
	struct pos *array = malloc(width * height * sizeof(struct pos));
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(buffer[y * width + x]  == '#')
			{
				array[n_nodes].x = x;
				array[n_nodes].y = y;
				n_nodes++;
			}
		}
	}
	struct pos *tmp = realloc(array, n_nodes * sizeof(struct pos));
	if(tmp != NULL)
		array = tmp;
	*out_n_nodes = n_nodes;
	return array;
}

// matrix of node-to-node distances
// matrix[a * n_nodes + b] is dist from a to b
// takes a distance function

// VERY IMPORTANT
// MATRIX IS DIRECTIONAL AND IS FROM A TO B
int *array_to_matrix(struct pos *array, int n_nodes, int (*dist)(struct pos, struct pos, char *heatmap, int width, int height), struct pos start_pos, int *start_vertex, char *heatmap, int width, int height)
{
	int *matrix = malloc(n_nodes * n_nodes * sizeof(int)); // could use short to save mem but nah
	for(int i = 0; i < n_nodes; i++)
	{
		if (array[i].x == start_pos.x && array[i].y == start_pos.y) // saves starting node index
			*start_vertex = i;
		for(int j = 0; j < n_nodes; j++)
		{
			matrix[i * n_nodes + j] = (*dist)(array[i], array[j], heatmap, width, height); // takes the dist from the ith to the jth elements and writes it in matrix[i][j]
		}
	}
	return matrix;
}

//---------------------------bicagis-code-END-----------------------------//

int distance(struct pos a, struct pos b, char *heatmap, int width, int height)
{
	double x = abs(b.x - a.x) - 1, y = a.y - b.y; // calculate the distance Pesho needs to jump
	
	if (x <= 0)
	{
		if (y <= 0) // if it's next to or below
			return 0;
		x = 0.0625; // this adds just a bit of differance so that the formula works when x is 0
		// it shouldn't be a problem, but if it is just make it even lower (not 0 of course) 
	}
	if (x + 1 < width && y <= height / 2 && y >= height / -2) 
		return heatmap[(int)(height / 2 - y) * width + (int)(x + 1)] - '0'; // if it's in the heatmap, return the value from the heatmap

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
	for (int i = 0; i < 10; i++) 	// currently the resolution is 1/1024 of a degree, it's determined by how long it iterates
	{ 				// so just change the 10 to whatever you want (don't forget it needs to fit in a double)
		offset /= 2;
		angleUp = angle + offset;
		angleDown = angle - offset;
		velocityUp = sqrt(gxx2 / (x * tan(angleUp) - y)) / cos (angleUp); // this is the formula for calculating the velocity as you saw 10 lines above
		velocityDown = sqrt(gxx2 / (x * tan(angleDown) - y)) / cos (angleDown); // took me 2 hours and desmos accoount to figure it out
		// this formula is good and all but it's going to be a nightmare to add air resistance
		if (velocityUp > velocityDown)
		{
			velocityMid = velocityDown;
			angle = angleDown;
		}
		else
		{
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

#define DIST distance

int pathFinder(char *map, int width, int height, int start_vertex, int destination, int *matrix, int n_nodes, struct pos *array, int *visited, int pesho)
{
	if (start_vertex == destination)
	{
		map[array[start_vertex].y * width + array[start_vertex].x] = 'X';
		return 1;
	}
	for (int i = 0; i < n_nodes; i++)
	{
		if (visited[i] == 0 && matrix[start_vertex * n_nodes + i] <= pesho)
		{
			visited[i] = 1;
			if (pathFinder(map, width, height, i, destination, matrix, n_nodes, array, visited, pesho))
			{
				map[array[start_vertex].y * width + array[start_vertex].x] = 'X';
				return 1;
			}
		}
	}
	return 0;
}

int dfs(int *matrix, int n_nodes, int start_vertex, int *requiredToReach) 
{
	for (int i = 0; i < n_nodes; i++)
	{
		if (i == start_vertex)
			continue;
		if (requiredToReach[i] > max(requiredToReach[start_vertex], matrix[start_vertex * n_nodes + i]))
		{
			requiredToReach[i] = max(requiredToReach[start_vertex], matrix[start_vertex * n_nodes + i]);
			dfs(matrix, n_nodes, i, requiredToReach);
		}
	}
}

void kill(char *buffer, struct pos *array, int *matrix) // should rename to murder
{
	free(buffer);
	free(array);
	free(matrix);
}

int n_digits(int n)
{
	return floor(log10(n) + 1);
}

void draw_map(const char *map, int width, int height)
{
	width--;
	int wn_digits = n_digits(width-1), hn_digits = n_digits(height-1);
	for (int i = pow(10, wn_digits - 1); i; i/=10) {
		// for (int j = 0; j < hn_digits; j++)
		// 	printf(" ");
		printf("0");
		for (int j = 1; j < width; j++) {
			if (j / i == 0)
			{
				printf(" ");
				continue;
			}
			printf("%d", (j / i) % 10);
		}
		printf("\n");
	}
	width++;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width - 1; x++)
			printf("%c", map[y * width + x]);
		printf("%d\n", y);
	}
}

int main() // also writtern by bicagis // you wish
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
	int start_vertex = -1;
	struct pos start_pos = {0, 0}; // added these so that I know where the start is
	
	// this is just for the looks
	printf("\n"); 
	draw_map(buffer, width, height); 
	printf("Show coordinates? (y/n): "); 
	char c; 
	scanf(" %c", &c); 
	if (c == 'y') 
	{ 
		for (int i = 0; i < n_nodes; i++) 
		{ 
			printf("%d: (%d,%d)\n", i, array[i].x, array[i].y); 
		} 
	} 
	
	printf("\nEnter Pesho's position (x,y): ");
	scanf("%d %d", &start_pos.x, &start_pos.y);
	FILE *heatmap = fopen("heatmap.txt", "r");
	int h_width, h_height;
	char *heatmap_buffer = file_to_buffer(heatmap, &h_width, &h_height);
	fclose(heatmap);
	int *matrix = array_to_matrix(array, n_nodes, &DIST, start_pos, &start_vertex, heatmap_buffer, h_width, h_height); // DIST - your distance function of choice, placeholder
//	free(array); // NOOO, don't take the array away from me, I need it
	if(start_vertex == -1)
	{
		printf("Pesho wasn't on a platform and fell to his death\n"); // :(  // the policemen also died, but the number of casulties is unknown since we haven't got to that part of the code yet
		kill(buffer, array, matrix);
		return 1;
	}
//	printf("file size: %d\n", fsize);
//	printf("width, height: %d,%d\n", width, height);
//	printf("number of #'s: %d\n", n_nodes);
//	if(n_nodes > 1)
//		printf("dist from 1st to 2nd #: %d\n", matrix[1 * n_nodes + 2]);

	// only thing that you care about is matrix and n_nodes beyond this point, that and DIST, also array_to_matrix() comment
// the following code worked on the second try 
// very proud of that
// at least I hope it does, I haven't tested it enough
	int *requiredToReach = malloc(n_nodes * sizeof(int)), pesho, policeman, max = 0, n_policemen;
	for (int i = 0; i < n_nodes; i++)
	{
		requiredToReach[i] = INT_MAX;
	}
	requiredToReach[start_vertex] = 0;
	printf("Enter Pesho's horizontal jump distance: ");
	scanf("%d", &pesho);
	printf("Enter # of policemen: ");
	scanf("%d", &n_policemen);
	printf("For each policeman, enter jump distance: ");
	for (int i = 0; i < n_policemen; i++)
	{
		scanf("%d", &policeman); // this whole thing is   p o i n t l e s s
		if (policeman > max) // 39 buried
			max = policeman; // 0 found
	}

	dfs(matrix, n_nodes, start_vertex, requiredToReach);
	if (max >= pesho)
	{
		printf("IMPOSSIBLE, Pesho can jump less than the police\n"); // :(
		kill(buffer, array, matrix);
		return 1;
	}
	int destination = -1;
	for (int i = 0; i < n_nodes; i++)
	{
		if (requiredToReach[i] <= pesho && requiredToReach[i] > max)
		{
			destination = i;
			break;
		}
	}
	if (destination == -1)
	{
		printf("IMPOSSIBLE, Pesho has no place to hide\n"); // :(
		kill(buffer, array, matrix);
		return 0;
	}
	int *visited = malloc(n_nodes * sizeof(int));
	for (int i = 0; i < n_nodes; i++)
	{
		visited[i] = 0;
	}
	visited[start_vertex] = 1;
	pathFinder(buffer, width, height, start_vertex, destination, matrix, n_nodes, array, visited, pesho);

	printf("\n");
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			printf("%c", buffer[y * width + x]);
		}
	}
	printf("\n");
	printf("PESHO ESCAPES AGAIN!\n"); // :)
//  ⠀⠀⠀⡯⡯⡾⠝⠘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢊⠘⡮⣣⠪⠢⡑⡌
//  ⠀⠀⠀⠟⠝⠈⠀⠀⠀⠡⠀⠠⢈⠠⢐⢠⢂⢔⣐⢄⡂⢔⠀⡁⢉⠸⢨⢑⠕⡌
//  ⠀⠀⡀⠁⠀⠀⠀⡀⢂⠡⠈⡔⣕⢮⣳⢯⣿⣻⣟⣯⣯⢷⣫⣆⡂⠀⠀⢐⠑⡌
//  ⢀⠠⠐⠈⠀⢀⢂⠢⡂⠕⡁⣝⢮⣳⢽⡽⣾⣻⣿⣯⡯⣟⣞⢾⢜⢆⠀⡀⠀⠪
//  ⣬⠂⠀⠀⢀⢂⢪⠨⢂⠥⣺⡪⣗⢗⣽⢽⡯⣿⣽⣷⢿⡽⡾⡽⣝⢎⠀⠀⠀⢡
//  ⣿⠀⠀⠀⢂⠢⢂⢥⢱⡹⣪⢞⡵⣻⡪⡯⡯⣟⡾⣿⣻⡽⣯⡻⣪⠧⠑⠀⠁⢐
//  ⣿⠀⠀⠀⠢⢑⠠⠑⠕⡝⡎⡗⡝⡎⣞⢽⡹⣕⢯⢻⠹⡹⢚⠝⡷⡽⡨⠀⠀⢔
//  ⣿⡯⠀⢈⠈⢄⠂⠂⠐⠀⠌⠠⢑⠱⡱⡱⡑⢔⠁⠀⡀⠐⠐⠐⡡⡹⣪⠀⠀⢘
//  ⣿⣽⠀⡀⡊⠀⠐⠨⠈⡁⠂⢈⠠⡱⡽⣷⡑⠁⠠⠑⠀⢉⢇⣤⢘⣪⢽⠀⢌⢎
//  ⣿⢾⠀⢌⠌⠀⡁⠢⠂⠐⡀⠀⢀⢳⢽⣽⡺⣨⢄⣑⢉⢃⢭⡲⣕⡭⣹⠠⢐⢗
//  ⣿⡗⠀⠢⠡⡱⡸⣔⢵⢱⢸⠈⠀⡪⣳⣳⢹⢜⡵⣱⢱⡱⣳⡹⣵⣻⢔⢅⢬⡷
//  ⣷⡇⡂⠡⡑⢕⢕⠕⡑⠡⢂⢊⢐⢕⡝⡮⡧⡳⣝⢴⡐⣁⠃⡫⡒⣕⢏⡮⣷⡟
//  ⣷⣻⣅⠑⢌⠢⠁⢐⠠⠑⡐⠐⠌⡪⠮⡫⠪⡪⡪⣺⢸⠰⠡⠠⠐⢱⠨⡪⡪⡰
//  ⣯⢷⣟⣇⡂⡂⡌⡀⠀⠁⡂⠅⠂⠀⡑⡄⢇⠇⢝⡨⡠⡁⢐⠠⢀⢪⡐⡜⡪⡊
//  ⣿⢽⡾⢹⡄⠕⡅⢇⠂⠑⣴⡬⣬⣬⣆⢮⣦⣷⣵⣷⡗⢃⢮⠱⡸⢰⢱⢸⢨⢌
//  ⣯⢯⣟⠸⣳⡅⠜⠔⡌⡐⠈⠻⠟⣿⢿⣿⣿⠿⡻⣃⠢⣱⡳⡱⡩⢢⠣⡃⠢⠁
//  ⡯⣟⣞⡇⡿⣽⡪⡘⡰⠨⢐⢀⠢⢢⢄⢤⣰⠼⡾⢕⢕⡵⣝⠎⢌⢪⠪⡘⡌⠀
//  ⡯⣳⠯⠚⢊⠡⡂⢂⠨⠊⠔⡑⠬⡸⣘⢬⢪⣪⡺⡼⣕⢯⢞⢕⢝⠎⢻⢼⣀⠀
//  ⠁⡂⠔⡁⡢⠣⢀⠢⠀⠅⠱⡐⡱⡘⡔⡕⡕⣲⡹⣎⡮⡏⡑⢜⢼⡱⢩⣗⣯⣟
//  ⢀⢂⢑⠀⡂⡃⠅⠊⢄⢑⠠⠑⢕⢕⢝⢮⢺⢕⢟⢮⢊⢢⢱⢄⠃⣇⣞⢞⣞⢾
//  ⢀⠢⡑⡀⢂⢊⠠⠁⡂⡐⠀⠅⡈⠪⠪⠪⠣⠫⠑⡁⢔⠕⣜⣜⢦⡰⡎⡯⡾⡽
	kill(buffer, array, matrix);
	return 0;
}
