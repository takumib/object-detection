#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define LOAD_RGB  CV_LOAD_IMAGE_COLOR
#define LOAD_GRAY CV_LOAD_IMAGE_GRAYSCALE

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)<(b))?(b):(a))

// compiled with
//gcc example.c -o example `pkg-config --cflags --libs opencv`

void bilateralKernel(IplImage *image);

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
	bilateralKernel(img);
	
	//Display the image on the window
	cvShowImage(window, img);

	cvSaveImage("object-detection-output.jpg", img, 0);

	//Wait key to signal exit  
	cvWaitKey(0);

	//Releases the image
	cvReleaseImage(&img);

	return 0;
}

void bilateralKernel(IplImage *image)
{
	int i, j, k, ii, jj, kk;
	int width, height, step, channels;
	unsigned char *src;
	unsigned char *dest;
	float colorDist, imageDist, colorSum, imageSum;
	int rows, cols, nbrs;

	height    = image->height;
	width     = image->width;
	step      = image->widthStep;
	channels  = image->nChannels;
	src      = (unsigned char *)image->imageData;
	
	dest = (unsigned char *)malloc(sizeof(unsigned char) * width * height * channels);
	
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			
			for(k = 0; k < channels; k++)
			{
				colorSum = imageSum = 0;
				nbrs = 0;

				for(ii = max(0,i-1); ii <= min(height-1,i+1); ii++)
				{
					cols = 0;
					for(jj = max(0, j-1); jj <= min(width-1, j+1); jj++)
					{
						//for(kk = 0; kk < channels; kk++)
						//{
							colorDist = src[ii*step+jj*channels+k] - src[i*step+j*channels+k]; 
							imageDist = sqrt((float)(((ii - i) * (ii - i)) + ((jj - j) * (jj - j))));
							colorSum += colorDist * colorDist;
							imageSum += imageDist; 
							nbrs++;
						//}
					}
					
				}
				float colorSumSqrd = sqrt(colorSum);
				float imageSumSqrd = sqrt(imageSum);
			//	printf("imageSum: %f\n", imageSum / (float)nbrs);
				//CvMat mat = cvMat(rows, cols, CV_8UC3, dest); 
				//CvMat *ptr = &mat;
				//double norm = cvNorm(image, NULL, CV_L2, NULL);
				src[i*step+j*channels+k] = (exp(((float)colorSumSqrd / (float)nbrs) * ((float)colorSumSqrd / (float)nbrs) * 0.5) * exp(((float)imageSum / (float)nbrs) * ((float)imageSum / (float)nbrs) * 0.5));
			}
		}
		//printf("done: %d", i);
	}

	//memcpy(src, dest, sizeof(unsigned char) * width * height); 
	
	//free(dest);
}




