#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define TYPE CV_8UC1
#define DEBUG 1

// compiled with
//gcc slidingwindow.c -c `pkg-config --cflags --libs opencv`

//Do we need to check the size of the image?

CvMat** window(IplImage * query, IplImage Test)
{
	//Get the sizes of the images
	int query_height = query->height;
	int query_width = query->width;

	//We want the size of the query image in bytes for data manipulation later
	int query_size = query_width * query_height * sizeof (unsigned char);
	int test_height = Test.height;
	int test_width = Test.width;

	//Get the data out of the test image so we can manipulate it 
	//Unsigned vs signed char?
	unsigned char * data = (unsigned char *)Test.imageData;
	int i,j,k,l;

	//FIXME This math may need work 
	//We need to figure out how many query patches we can fit in vertically and horizontally
	//The values are equivilant in a 2^n image, but why not be general?
	int numhoriz = test_width - (query_height/2);
	int numvert = test_height - (query_height/2);

	//Allocate memory for each of the patches
	CvMat ** patches = malloc(numhoriz * numvert * sizeof(unsigned char) * sizeof(CvMat));

	//Allocate memory for a swap patch
	unsigned char ** swap = malloc(query_height * query_width * sizeof(unsigned char));

	//We need to keep track of how many patches we have sampled
	int patchindex = 0;

	//For each horizontal space
	for(i = 0; i < numhoriz; i++)
	{
		//and each vertical space
		for(j = 0; j < numvert; j++)
		{

			//Create a new patch
			patches[patchindex] = cvCreateMat(query_height, query_width, TYPE);

			//For each pixel of the patch...
			for(k = i * numhoriz; k < query_width; k++)
			{
				for(l = j * numvert; l < query_height; l++)
				{
					//Get a data value
					swap[k][l] = data[k * query_height + l];
				}
			}
			//FIXME figure out how to get the data from the swap into the array
			memcpy(patches[patchindex]->data.ptr, swap, query_size);
			patchindex+=1;
		}

	}	

	return patches;	
}
