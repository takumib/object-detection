#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define TYPE CV_8UC1
#define DEBUG 1

// compiled with
//gcc slidingwindow.c -c `pkg-config --cflags --libs opencv`


CvMat** get_queries(IplImage * Test, IplImage * query);

int main(int argc, char *argv[])
{
	int height, width, step, channels;
	unsigned char *data;
	char *window = "Object Detection";
	int i, j = 0;
	CvMat stub,* query_mat;


	//If we do not have an input image
	if(argc < 3)
	{
		printf("Usage: object-detection <image> <query image>\n");
		exit(0);
	}

	//Load image from input
	IplImage *src = NULL;
	IplImage *query = NULL;
	src =  cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	query = cvLoadImage(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
	//CvCapture *capture = cvCaptureFromCAM(0);

	if(!src)
	{
		printf("Could not load image file: %s\n", argv[1]);
		exit(0);
	}

	//Get the image data
	height    = src->height;
	width     = src->width;
	step      = src->widthStep;
	channels  = src->nChannels;
	data      = (unsigned char *)src->imageData;


	//Information about the image
	printf("Processing a %dx%d image with %d channels\n", height, width, channels); 

	//Set up basic window
	cvNamedWindow(window, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(window, 100, 100);

	printf("starting the patching process\n");

	//get the patches
	CvMat ** patches = get_queries(src, query);
	IplImage * temp;
	IplImage * buffer;
	buffer =  cvCreateImage(cvSize(query->width,query->height), IPL_DEPTH_8U, 1);

	printf("Patches done, %d of them %d wide\n",(src->height - (query->height-1)), patches[0]->rows );

	cvShowImage(window,src);
	cvWaitKey(0);
	for(i = 0; i < (src->height - (query->height-1));i++)
	{
		printf("%d, %d, %d\n",patches[i]->data.i[0],patches[i]->data.i[1],patches[i]->data.i[2]);
		temp = cvGetImage(patches[i], buffer);
		cvShowImage(window, temp);			
		//cvWaitKey(0);
	}


	cvSaveImage("object-detection-output.jpg",temp, 0);

	//Wait key to signal exit  
	cvWaitKey(0);

	//Releases the image
	//cvReleaseCapture(&capture);
	return 0;
}



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
			patches[patchindex] = cvCreateMat(query_height, query_width, CV_8UC1);

			//For each pixel in the original image
			for(k = 0; k < query_width; k++)
			{
				for(l = 0; l < query_height; l++)
				{
					//Get a data value and put it in the swap space
					swap[k][l] = data[(i * query_width + j) + k * query_width + l];
					//		printf("[%d]",swap[k][l]);
				}
				//	printf("\n");
			}
			//printf("\n\n\n");
			//Copy the swap data into the patch 
			memcpy(patches[patchindex]->data.ptr, swap, query_size);

			//keep track of which patch we are looking at 
			patchindex+=1;


			//This code shows the matrix 
			/*
			   CvMat * temp =  patches[patchindex - 1];
			   unsigned char * temp2 = temp->data.ptr;

			   printf("\n\n\n %d",patchindex);
			   for(k = 0;k < query_width; k++)
			   {
			   for(l = 0; l < query_height; l++)
			   {
			//		printf("[%d]",data[(i * query_width + j) + k * query_width + l]);
			printf("[%d]", temp2[k*query_width + l]);				
			}

			printf("\n");
			} 
			 */

			//			cvWaitKey(0);

		}
	}	
	return patches;	
}
