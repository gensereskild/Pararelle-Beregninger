#ifndef BITMAP_H
#define BITMAP_H


typedef unsigned char uchar;

void savebmp(char *name, uchar *buffer, int x, int y);
void readbmp(char *filename, uchar *array);

void InvertBitmap(uchar *bitmap, int XSIZE, int YSIZE);
void ChangeColor(char * farge, uchar *bitmap, int XSIZE, int YSIZE);

uchar* DoubleResolution(uchar *bitmap, int XSIZE, int YSIZE);


#endif
