#include <stdlib.h>
#include <stdio.h>
#include "bitmap.h"

#include "bitmap.c"

#define XSIZE 2560 // Size of before image
#define YSIZE 2048



int main(void)
{
	uchar *image = calloc(XSIZE * YSIZE * 3, 1); // Three uchars per pixel (RGB)
	readbmp("before.bmp", image);

	// Alter the image here
	printf("\nEndrer bilde\n");

	//InvertBitmap(image,XSIZE,YSIZE);
	//ChangeColor("red", image,XSIZE,YSIZE);

	uchar *DoubleImage = DoubleResolution(image,XSIZE,YSIZE);
	;

	savebmp("after.bmp", DoubleImage, XSIZE*2, YSIZE*2);

	free(image);
	free(DoubleImage);
	return 0;
}
