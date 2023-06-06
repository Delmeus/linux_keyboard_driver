#include <stdio.h>

int main() {
	FILE *fptr;
   	fptr = fopen("/home/antek/konami_egg.txt","w");
   	fprintf(fptr, "Gratulacje, znalazles easter egga, oto nagroda:\nhttps://www.youtube.com/watch?v=dQw4w9WgXcQ\n");
   	fclose(fptr);
   	
    	return 0;
}

