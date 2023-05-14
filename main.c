#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

struct pos
{
	int x, y;
};

struct node
{
	
	pos *cells;
};

int file_size(FILE *file) //in bytes
{
	fseek(file,0,SEEK_END); //sets file position to EOF
	int rval = (int)ftell(file); //we assume the file is smaller than INT_MAX
	rewind(file);
	return rval;
}

char* file_to_matrix(FILE *file, int *width, int *height) // must be a rectangle, else it doesnt work
{
	char *buffer = malloc(file_size(file));

	fgets(buffer, INT_MAX, file);
	*width = strlen(buffer);
	*height = file_size(file) / *width;

	for(int i = 0; i < *height; i++)
	{
		fgets(buffer + i * *width, INT_MAX, file);
	}
	return buffer;
}



void main()
{
	printf("Cock\n");
	FILE *map = fopen("map.txt", "r");
	printf("%d\n", file_size(map));
	int w,h;
	char *c = file_to_matrix(map, &w, &h);
	printf("%s,%d,%d",c,w,h);
	fclose(map);
}


	///fwrite()
	///fread(), fseek(), fprintf()
