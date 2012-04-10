#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define LOAD_RGB CV_LOAD_IMAGE_COLOR


// compiled with
//gcc example.c -o example `pkg-config --cflags --libs opencv`
// on my mac

// code source from http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/

int main(int argc, char *argv[])
{
	int height, width, step, channels;
	unsigned char *data;
	char *window = "Object Detection";
	int i, j, k;

	//If we do not have an input image
	if(argc < 2)
	{
		printf("Usage: object-detection <image-file>\n");
		exit(0);
	}

	//Load image from input
	IplImage* img = 0;
	img =  cvLoadImage(argv[1], LOAD_RGB);

	if(!img)
	{
		printf("Could not load image file: %s\n", argv[1]);
		exit(0);
	}
	
	//Get the image data
	height    = img->height;
	width     = img->width;
	step      = img->widthStep;
	channels  = img->nChannels;
	data      = (unsigned char *)img->imageData;

	//Information about the image
	printf("Processing a %dx%d image with %d channels\n", height, width, channels); 

	//Set up basic window
	cvNamedWindow(window, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(window, 100, 100);
	
	//Invert the image
	for(i =0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				data[i*step+j*channels+k] = 255 - data[i*step+j*channels+k];
			}
		}
	}

	//Display the image on the window
	cvShowImage(window, img);


	cvSaveImage("object-detection-output.jpg", img, 0);

	//Wait key to signal exit  
	cvWaitKey(0);

	//Releases the image
	cvReleaseImage(&img);

	return 0;
}


