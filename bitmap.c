#include <stdlib.h>
#include <stdio.h>
#include "bitmap.h"






// save 24-bits bmp file, buffer must be in bmp format: upside-down
void savebmp(char *name, uchar *buffer, int x, int y) {
	FILE *f=fopen(name,"wb");
	if(!f) {
		printf("Error writing image to disk.\n");
		return;
	}
	unsigned int size=x*y*3+54;
	uchar header[54]={'B','M',size&255,(size>>8)&255,(size>>16)&255,size>>24,0,
                    0,0,0,54,0,0,0,40,0,0,0,x&255,x>>8,0,0,y&255,y>>8,0,0,1,0,24,0,0,0,0,0,0,
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	fwrite(header,1,54,f);
	fwrite(buffer,1,x*y*3,f);
	fclose(f);
}


// read bmp file and store image in contiguous array
void readbmp(char* filename, uchar* array) {
	FILE* img = fopen(filename, "rb");   //read the file
	uchar header[54];
	size_t result = fread(header, sizeof(uchar), 54, img); // read the 54-byte header

	//Det her gir jo null mening man leser 54 bytes
	//og sjekker om man har lest 54 bytes???
	//Det forteller ingenting om headern er feil?
	//Det gir bare feil dersom filen er kortere enn 
	//54 bytes
	if (result != 54)
	{
		perror("Error reading file.\n");
		return;
	}

  // extract image height and width from header
	int width = *(int*)&header[18];
	int height = *(int*)&header[22];
	int padding=0;
	while ((width*3+padding) % 4!=0) padding++;

	int widthnew=width*3+padding;
	uchar* data = calloc(widthnew, sizeof(uchar));

	for (int i=0; i<height; i++ ) {
		result = fread( data, sizeof(uchar), widthnew, img);

		if (result != (size_t) widthnew)
		{
			perror("Error reading file.\n");
		}

		for (int j=0; j<width*3; j+=3) {
			array[3 * i * width + j + 0] = data[j+0];
			array[3 * i * width + j + 1] = data[j+1];
			array[3 * i * width + j + 2] = data[j+2];
		}
	}
	fclose(img); //close the file
}

//Tar inn pointer til bitmap og inverterer alle fargene
//Tar også inn XSIZE og YSIZE
//Når man tar en uchar pointer er pointeren framleis
//1 byte, nei må være variablene den lagrer
//pointer er vel alltid 8 bytes tror jeg
void InvertBitmap(uchar *bitmap, int XSIZE, int YSIZE){
	//Leser
	uchar farge = 0;
	for(int y=0; y < YSIZE; y++){
		for(int x=0; x<XSIZE; x++){
			for(int byte = 0; byte < 3; byte++){
				farge = bitmap[y*XSIZE*3 + x*3 + byte];
				farge = 255-farge;
				bitmap[y*XSIZE*3 + x*3 + byte] = farge;
			}
		}
	}
}
//Funksjon som konverterer fra lysstyrke til spesifisert
//farge. + alle fargene / 3 setter til farge.
void ChangeColor(char *farge, uchar *bitmap, int XSIZE, int YSIZE){
	unsigned int lysstyrke = 0;

	for(int y=0; y < YSIZE; y++){
		for(int x=0; x<XSIZE; x++){
			lysstyrke=0;
			for(int byte = 0; byte < 3; byte++){
				lysstyrke += bitmap[y*XSIZE*3 + x*3 + byte];
				bitmap[y*XSIZE*3 + x*3 + byte] = 0;
			}
			if(farge == "blue"){
				bitmap[y*XSIZE*3 + x*3 + 0] = lysstyrke/3;
			}if(farge == "green"){
				bitmap[y*XSIZE*3 + x*3 + 1] = lysstyrke/3;
			}if(farge == "red"){
				bitmap[y*XSIZE*3 + x*3 + 2] = lysstyrke/3;
			}
		}
	}
}

//Ikke helt rett fram...
//først duplikerer piksel til høyre, så duplikerer linjen et steg opp.
//oppløsningen blit større men piksele også større så ikke mer detalj.
//Må også endre bitmap header... steg 1: hvordan få tak i headeren?
//Det er enklere om jeg lager ny bitmap istedefor å endre den in place.
//Må ha mer minnestørrelse etc uansett.
uchar* DoubleResolution(uchar *bitmap, int XSIZE, int YSIZE){
	//ganger med 4 fordi 2x lengde og 2 x bredde.
	printf("\n før calloc");
	uchar *doubleBitmap = calloc(YSIZE*XSIZE*4*3, 1);
	printf("\n verdi til doubleBitmap %p", doubleBitmap);
	printf("\n før bitmap \n");
	fflush(stdout);
	uchar farge = 0;

	//Skriver 2 linjer samtidig.

	for(int y=0; y < YSIZE; y++){
		for(int x=0; x<XSIZE; x++){
			for(int byte = 0; byte < 3; byte++){
				farge = bitmap[y*XSIZE*3 + x*3 + byte];
				doubleBitmap[y*4*XSIZE*3 + x*2*3 + byte] = farge;
				doubleBitmap[y*4*XSIZE*3 + x*2*3 + byte +3] = farge;

				doubleBitmap[(y)*4*XSIZE*3 + (XSIZE*3*2) + x*2*3 + byte] = farge;
				doubleBitmap[(y)*4*XSIZE*3 + (XSIZE*3*2) + x*2*3 + byte +3] = farge;
			}
		}
	}
	printf("\n ferdig bitmap \n");
	fflush(stdout);
	return doubleBitmap;

}