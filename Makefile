main: main.c
	gcc $^ -lm -g -o $@
clean:
	rm *.o
