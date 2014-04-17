#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE * InputFile;
#define BufSize 256
char buf[BufSize][BufSize];
char addres[BufSize] = "C:\\BricxCC\\API\\";

int main (int argc, char* argv[]) {

	InputFile = fopen (argv[1], "r+" );
	if (InputFile==NULL) {printf("Error opening InputFile\n"); exit (1);}

	int line = 0;
	while(fscanf(InputFile, "%s", buf[line]) != EOF){
		line++;
	}

	rewind(InputFile);

	int eind = line;
	line = 0;
	while(line<eind){
		fprintf(InputFile, "%s%s\n", addres, buf[line]);
		line++;
	}
	
	fclose (InputFile);
	return 0;
}

