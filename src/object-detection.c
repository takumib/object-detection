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
void init_mat(char *mat, int n);
void pca(IplImage *image);


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
	pca(img);
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
							imageDist = (float)(((ii - i) * (ii - i)) + ((jj - j) * (jj - j)));
							colorSum += colorDist * colorDist;
							imageSum += imageDist; 
							nbrs++;
						//}
					}
					
				}
				float colorSumSqrd = sqrt(colorSum);
				float imageSumSqrd = sqrt(imageSum);
			//	printf("imageSum: %f\n", imageSum / (float)nbrs);
				float colorSqrd = ((float)colorSumSqrd / (float)nbrs) * ((float)colorSumSqrd / (float)nbrs);
				float imageSqrd = ((float)imageSumSqrd / (float)nbrs) * ((float)imageSumSqrd / (float)nbrs);	
				src[i*step+j*channels+k] = exp(colorSqrd * 0.5) - exp(imageSqrd * 0.5);
				//float bilat = ((colorSqrd - imageSqrd) / (float)nbrs);
				//src[i*step+j*channels+k] = exp(-0.5 * (bilat * bilat));
			}
		}
		//printf("done: %d", i);
	}

	//memcpy(src, dest, sizeof(unsigned char) * width * height); 
	
	//free(dest);
}

void pca(IplImage *image)
{
	int i, j, k;
	int width, height, step, channels;
	unsigned char *src;
	unsigned char *u;
	unsigned char *b;//, *bg, *bb;
	unsigned char *g;

	height    = image->height;
	width     = image->width;
	step      = image->widthStep;
	channels  = image->nChannels;
	src       = (unsigned char *)image->imageData;
	
	u = (unsigned char *)malloc(sizeof(unsigned char) * height * channels);
	b = (unsigned char *)malloc(sizeof(unsigned char) * width * height * channels);
	g = (unsigned char *)malloc(sizeof(unsigned char) * height * channels);
	
	//bg = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
	//bb = (unsigned char *)malloc(sizeof(unsigned char) * width * height);

	init_mat(u, height * channels);
	init_mat(b, height * width);
	//init_mat(bg, height * width);
	//init_mat(bb, height * widht);
	init_mat(g, height * channels);

	//begin by calculating the empirical mean
	//u[1..height] = 1/n sum(src[i,j])
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				u[i*channels + k] += src[i*step+j*channels+k] / width; 
			}
		}
	}

	printf("empirical means working\n\n");

	//we next calculate the deviation from the mean
	//b = src[i,j] - u;
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				b[i*step+j*channels+k] -= u[i*channels+k];
				//bg[i*step+j] -= u[i*channels+1];
				//bb[i*step+j] -= u[i*channels+2];
			}	
		}
	}
	
	printf("deviation working\n\n");

	//we now need to find the covariance matrix
	
	//b in opencv matrix form
	CvMat bMat = cvMat(height, width, CV_8UC3, b);
	//CvMat bgMat = cvMat(height, width, CV_8U3, bg);
	//CvMat bbMat = cvMat(height, width, CV_8U3, bb);

	//b transpose
	CvMat bT = cvMat(width, height, CV_8UC3, b);	
	//CvMat *bgT = cvCreateMat(height, width, CV_8UC3);	
	//CvMat *bbT = cvCreateMat(height, width, CV_8UC3);	

printf("Transposing cross product\n");	
	cvTranspose(&bMat, &bT);
	//cvTranspose(&bgMat, bgT);
	//cvTranspose(&bbMat, bbT);
	
	//covariance matrix
	CvMat *c = cvCreateMat(height, height, CV_8UC3);
	//CvMat *cg = cvCreaetMat(height, height, CV_8UC3);
	//CvMat *cb = cvCreateMat(height, height, CV_8UC3);
	//printf("%d\n", &bMat->cols);
	//printf("%d\n", &bMat->rows);

printf("Doing that cross product\n");
	cvCrossProduct(&bMat, &bT, c);
	//cvCrossProduct(&bMat, &bT, &c);
	//cvCrossProduct(&bgMat, &bgT, &cg);	
	//cvCrossProduct(&bbMat, &bbT, &cb);

	printf("cross product working\n\n");

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < height; j++)
		{	
			cvmSet(c, i, j, cvmGet(c, i, j) / width); 
			//cvmSet(cg, i, j, cvmGet(cg, i, j) / width); 
			//cvmSet(cb, i, j, cvmGet(cb, i, j) / width); 
		}
	}	

	//eigenvector and values
        CvMat *eMat = cvCreateMat(height, height, CV_8UC3);
	CvMat *lMat = cvCreateMat(height, 1, CV_8UC3);

	cvEigenVV(&c, &eMat, &lMat, 0.0000001, -1, -1);

	unsigned char *l = lMat->data.ptr;
	unsigned char *e = eMat->data.ptr;
	
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < i+1; j++)
		{
			for(k = 0; k < channels; k++)
			{
				g[i*channels+k] += l[i*channels+k];
			}
		}
	}	

	int L = 0;
	float currVal = 0.0;

	for(i = 0; i < height; i++)
	{
		if(currVal >= 0.9)
		{
			L = i;
			break;
		}

		for(k = 0; k < channels; k++)
		{
			currVal += (float)g[i*channels+k] / (float)g[height - 1 + k];		
		}
	}

	unsigned char *w;
	
	w = (unsigned char *)malloc(sizeof(unsigned char) * height * L * channels);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < L; j++)
		{
			for(k = 0; k < channels; k++)
			{	
				w[i*step+j*channels+k] = e[i*step+j*channels+k];
			}
		}
	}

	unsigned char *s;
	s = (unsigned char *)malloc(sizeof(unsigned char) * height * channels);

	for(i = 0; i < height; i++)
	{
		for(k = 0; k < channels; k++)
		{
			s[i*channels+k] = sqrt(cvmGet(c,i,i));
		}
	}

	unsigned char *z;
	z = (unsigned char *)malloc(sizeof(unsigned char) * height * width * channels);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				z[i*step+j*channels+k] = (float)b[i*step+j*channels+k] / (float)s[i*channels+k];
			}
		}
	}

	CvMat wMat = cvMat(height, L, CV_8UC3, w);
	CvMat *wMatT = cvCreateMat(L, height, CV_8UC3); 

	cvTranspose(&wMat, &wMatT);

	CvMat zMat = cvMat(height, width, CV_8UC3, z);

	CvMat *yMat = cvCreateMat(height, width, CV_8UC3);

	cvCrossProduct(&wMatT, &zMat, &yMat); 
}

void init_mat(char *mat, int n)
{
	int i;

	for(i = 0; i < n; i++)
	{
		mat[i] = 0;
	}
}


