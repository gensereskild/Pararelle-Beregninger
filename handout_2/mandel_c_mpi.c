#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mpi.h>

#define XSIZE 2560
#define YSIZE 2048

//Øker max iteration fra 255
#define MAXITER 1000

double xleft=-2.01;
double xright=1;
double yupper,ylower;
double ycenter=1e-6;
double step;

int pixel[XSIZE*YSIZE];

#define PIXEL(i,j) ((i)+(j)*XSIZE)

typedef struct {
	double real,imag;
} complex_t;

void calculate(int y_start, int y_stop) {
	for(int i=0;i<XSIZE;i++) {
		for(int j=y_start;j<y_stop;j++) {
			/* Calculate the number of iterations until divergence for each pixel.
			   If divergence never happens, return MAXITER */
			complex_t c,z,temp;
			int iter=0;
			c.real = (xleft + step*i);
			c.imag = (ylower + step*j);
			z = c;
			while(z.real*z.real + z.imag*z.imag < 4) {
				temp.real = z.real*z.real - z.imag*z.imag + c.real;
				temp.imag = 2*z.real*z.imag + c.imag;
				z = temp;
				if(++iter==MAXITER) break;
			}
			pixel[PIXEL(i,j)]=iter;
		}
	}
}

typedef unsigned char uchar;

/* save 24-bits bmp file, buffer must be in bmp format: upside-down */
void savebmp(char *name,uchar *buffer,int x,int y) {
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
	fwrite(buffer,1,XSIZE*YSIZE*3,f);
	fclose(f);
}

/* given iteration number, set a colour */
void fancycolour(uchar *p,int iter) {
	if(iter==MAXITER);
	else if(iter<8) { p[0]=128+iter*16; p[1]=p[2]=0; }
	else if(iter<24) { p[0]=255; p[1]=p[2]=(iter-8)*16; }
	else if(iter<160) { p[0]=p[1]=255-(iter-24)*2; p[2]=255; }
	else { p[0]=p[1]=(iter-160)*2; p[2]=255-(iter-160)*2; }
}

int main(int argc,char **argv) {

	MPI_Init(&argc, &argv);

	int rank, size;
	
	MPI_Comm_rank( MPI_COMM_WORLD , &rank);
	MPI_Comm_size( MPI_COMM_WORLD , &size);

	if(argc==1 && rank == 0) {
		puts("Usage: MANDEL n");
		puts("n decides whether image should be written to disk (1=yes, 0=no)");
		return 0;
	}

	//Vi skal alltid ha 3 prosesser
	if(size!= 3){
		puts("Skal være 3 prosesser");
		return 0;
	}
	//Må dele inn i 3 rader før jeg kaller calculate

	/* Calculate the range in the y-axis such that we preserve the
	aspect ratio */
	step=(xright-xleft)/XSIZE;
	yupper=ycenter+(step*YSIZE)/2;
	ylower=ycenter-(step*YSIZE)/2;


	//Helt tilbakestående lmao i forgor divisjon i C ):
	if(rank == 0){
		calculate(0, YSIZE/3);
	}
	if(rank == 1){
		calculate(YSIZE/3, (YSIZE*2)/3);
	}
	if(rank == 2){
		calculate((YSIZE*2)/3, YSIZE);
	}

	int *Result_from_rank_1;
	int *Result_from_rank_2;
	if(rank == 0){
		Result_from_rank_1 = malloc(XSIZE*YSIZE * sizeof(int));
		Result_from_rank_2 = malloc(XSIZE*YSIZE * sizeof(int));
	}

	if(rank == 1){
		MPI_Send( pixel , XSIZE*YSIZE , MPI_INT , 0 , 0 , MPI_COMM_WORLD);
	}

	if(rank == 2){
		MPI_Send( pixel , XSIZE*YSIZE , MPI_INT , 0 , 1 , MPI_COMM_WORLD);
	}

	if(rank== 0){
		MPI_Recv( Result_from_rank_1 , XSIZE*YSIZE , MPI_INT , 1 , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		MPI_Recv( Result_from_rank_2 , XSIZE*YSIZE , MPI_INT , 2 , 1 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);

		printf("Brude ha fått resultatet \n");

		for(int i = 0; i<XSIZE; i++){
			for( int j  = YSIZE/3; j < (YSIZE*2)/3; j++){
				pixel[PIXEL(i, j)] = Result_from_rank_1[i + j*XSIZE];
			}
		}
		for(int i = 0; i<XSIZE; i++){
			for( int j  = (YSIZE*2)/3; j < YSIZE; j++){
				pixel[PIXEL(i, j)] = Result_from_rank_2[i + j*XSIZE];
			}
		}
	}

	if((strtol(argv[1],NULL,10)!=0)) {
		/* create nice image from iteration counts. take care to create it upside
		   down (bmp format) */
		unsigned char *buffer=calloc(XSIZE*YSIZE*3,1);
		for(int i=0;i<XSIZE;i++) {
			for(int j=0;j<YSIZE;j++) {
				int p=((YSIZE-j-1)*XSIZE+i)*3;
				fancycolour(buffer+p,pixel[PIXEL(i,j)]);
			}
		}
		/* write image to disk */
		if(rank == 0){
			savebmp("mandel0_mpi.bmp",buffer,XSIZE,YSIZE);
		}
	}
	MPI_Finalize();

	return 0;
}
