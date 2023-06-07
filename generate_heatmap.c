#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <float.h>

struct pos
{
	int x, y;
};


int func(struct pos a, struct pos b)
{
	double x = abs(b.x - a.x) - 1, y = a.y - b.y; // calculate the distance Pesho needs to jump
	
	if (x <= 0)
	{
		if (y <= 0) // if it's next to or below
			return 0;
		x = 0.0625; // this adds just a bit of differance so that the formula works when x is 0
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

int main()
{
    int size_x, size_y;
    scanf("%d %d", &size_x, &size_y);
    char *map = malloc(size_x * size_y * sizeof(char));
    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {
            map[y * size_x + x] = func((struct pos){0, 0} , (struct pos){x, -(size_y / 2 - y)});
        }
    }
    // write the map into heatmap.txt
    FILE *f = fopen("heatmap.txt", "w");
    for (int y = 0; y < size_y; y++)
    {
        for (int x = 0; x < size_x; x++)
        {
            fprintf(f, "%c", map[y * size_x + x] + '0');
        }
        fprintf(f, "\n");
    }
    fclose(f);
    free(map);
    return 0;
}