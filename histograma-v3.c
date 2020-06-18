#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

int getImageInfo(FILE*, int);
void *compute(void*);

int getImageInfo(FILE*, int);

int numthreads = 1, bread;
unsigned char *pChar;
long int NumPxls;
float hist[256];
long int **iHist;




int main(int argc, char* argv[])
{
  FILE *histogramData, *bmpInput;
  int rows, cols, nColors; 
  int r, c, i, j;


  struct timeval t1,t2;

  if(argc < 2)
  {
    printf("Usage: %s bmpInput.bmp\n", argv[0]);
    exit(0);
  } else if (argv == 2) {
	printf("Reading filename %s\n", argv[1]);
  } else if (argc == 3){
		printf("Reading filename %s\n", argv[1]);
		numthreads = atoi(argv[2]);
  }	
  	pthread_t th[numthreads];
	int id[numthreads];
	
	iHist = (long int **) calloc(numthreads ,sizeof(long int *));
	for (i = 0; i < numthreads; i++){
		iHist[i] = (long int *) calloc(256, sizeof(long int));
	}
	
  

  /*-----DECLARE INPUT AND OUTPUT FILES-------*/
  bmpInput = fopen(argv[1], "rb");
  histogramData = fopen("histData.txt", "w");

  fseek(bmpInput, 0L, SEEK_END);

  /*------READ INPUT IMAGE DATA------*/
  cols = abs(getImageInfo(bmpInput, 18));
  rows = abs(getImageInfo(bmpInput, 22));
  nColors = getImageInfo(bmpInput, 46);
  if (nColors==0) nColors=256;
  NumPxls = cols*rows;

  printf("Imagen de %dx%d con nColors=%d.\n",cols,rows,nColors);
  printf("Total # of pixels: %ld\n", NumPxls);

  /*------INITIALIZE ARRAY------*/
  for(i=0; i<256; i++)
  {
    hist[i] = 0;
  }

  pChar=(unsigned char*)calloc(NumPxls,sizeof(unsigned char));

  /*-----READ IMAGE DATA------*/
  fseek(bmpInput, (54 + 4*nColors), SEEK_SET);
  fread(pChar, sizeof(char), NumPxls, bmpInput);

  /*-----COMPUTE HISTOGRAM------*/
  gettimeofday(&t1,NULL);
  
  for (i=0; i<numthreads; i++)
	{
		id[i] = i;
		pthread_create(&th[i], NULL, compute, &id[i]);
	}

  for (i=0; i<numthreads; i++)
	pthread_join(th[i], NULL);
	
  for(i=0; i<256; i++){
	for(j=0; j < numthreads; j++){
	  hist[i] += iHist[j][i];
	}
	 hist[i] /= (float)NumPxls;
	}

  gettimeofday(&t2,NULL);

  /*-----PRINT HISTOGRAM AND TIME------*/
  for(i=0; i<256; i++)
	fprintf(histogramData, "%d\t%f\n", i, hist[i]);

	
	printf("El programa paralelo v3 con %d threads ha tardado %f segundos\n", numthreads, (t2.tv_sec-t1.tv_sec)+(t2.tv_usec-t1.tv_usec)/(double)1000000);
}

/*----------GET IMAGE INFO SUBPROGRAM--------------*/
int getImageInfo(FILE* inputFile, int offset)
{
  int *ptrC;
  int value = 0;

  ptrC=&value;	
  fseek(inputFile, offset, SEEK_SET);
  fread(ptrC, 4, 1, inputFile);

  return(value);
}

void *compute(void *arg){
	int myid, i;
	myid = *(int *) arg;
	
	for (i=myid*NumPxls/numthreads; i<(myid+1)*NumPxls/numthreads; i++){
		iHist[myid][pChar[i]]++;
	}
}
