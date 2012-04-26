#include <stdlib.h>
#include <cv.h>
#include <stdio.h>
#include "cosinesimilarity.h"
#define DEBUG 1

//CvScalar cosine_similarity(CvMat test, CvMat patch);
//int Resemblance(CvScalar map);


void mmm(char * t, char * q, char * dest, int N);

//Do the cosine similarity operation
//See: Frobenius inner product
//Boils down to the normalized product of the matrices
CvScalar cosine_similarity(CvMat * test, CvMat * patch)
{
	//Create a destination matrix
	CvMat* dest = cvCreateMat(patch->rows,patch->cols,patch->type);


	//Take the product of the two matrices 
	mmm(test->data.ptr,test->data.ptr,dest->data.ptr,patch->cols);

	//Trace the matrices 
	CvScalar scale = cvTrace(dest); 
	return scale;
}

//int?
double Resemblance(CvScalar map)
{
	double result = (int)(map.val[0] * map.val[0])/(1-(map.val[0]*map.val[0]));
	return result;
} 

void significance(resembleList res)
{
	int i;

	for(i = 0; i < res.size;i++){
		if(res.resemblences[i] >= TOLERANCE){
			res.resemblences[i] = 0.0;		
		}
	}
}
void mmm(char * t, char * q, char * dest, int N)
{
	int i,k,j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 0; k < N; k++) {
				dest[i * N + j] += t[i * N + k] * q[k * N +j];
			}   
		}   
	} 
} 

