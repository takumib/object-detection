#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include "window.h"

#define TYPE CV_8UC1
#define DEBUG 1

// compiled with
//gcc slidingwindow.c -c `pkg-config --cflags --libs opencv`

//This divides the image into peices 
CvMat** get_queries(IplImage * Test, IplImage * query)
{
	//Get the sizes of the images
	int query_height = query->height;
	int query_width = query->width;

	//We want the size of the query image in bytes for data manipulation later
	int query_size = query_width * query_height * sizeof (unsigned char);
	int test_height = Test->height;
	int test_width = Test->width;

	//Get the data out of the test image so we can manipulate it 
	//Unsigned vs signed char?
	unsigned char * data = (unsigned char *)Test->imageData;
	int i,j,k,l;

	//We need to figure out how many query patches we can fit in vertically and horizontally
	int numhoriz = test_width -  query_width + 1;
	int numvert = test_height - query_height + 1;

	//printf("Allocating patches with %d\n",numhoriz * numvert * sizeof(unsigned char) * sizeof(CvMat));

	//Allocate the container for the patches 
	CvMat ** patches = malloc(numhoriz * numvert * sizeof(unsigned char) * sizeof(CvMat));

	//Allocate memory for a swap patch
	printf("Allocating swap space h: %d  w: %d\n",query_height, query_width); 
	unsigned char ** swap = malloc(query_height * sizeof(char *));
	for(i = 0; i < query_width; i++)
	{
		swap[i] = malloc(query_width * sizeof(char));
	}

	//We need to keep track of how many patches we have sampled
	int patchindex = 0;

	//For each horizontal space
	for(i = 0; i < numhoriz; i++)
	{
		//and each vertical space
		for(j = 0; j < numvert; j++)
		{


			//Create a new patch
		//	printf("Patch values pi%d  qh %d, qw %d, %d\n", patchindex, query_height, query_width, CV_8UC1);
			patches[patchindex] = cvCreateMat(query_height, query_width, CV_8UC1);

			//For each pixel of the patch...
			for(k = 0; k < query_width; k++)
			{
				for(l = 0; l < query_height; l++)
				{
					//Get a data value
					swap[k][l] = data[(i * query_width + j) + k * query_width + l];
				}
			}
			printf("Data copying done\n");
			//FIXME figure out how to get the data from the swap into the array
			memcpy(patches[patchindex]->data.ptr, swap, query_size);
			patchindex+=1;

			CvMat * temp =  patches[patchindex - 1];
			unsigned char * temp2 = temp->data.ptr;

			/*printf("\n");
			for(k = 0;k < query_width; k++)
			{
				for(l = 0; l < query_height; l++)
				{
					printf("[%d]",data[(i * query_width + j) + k * query_width + l]);
				}

				printf("\n");
			} */ 
		}

	}	

	return patches;	
}
