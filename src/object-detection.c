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

//rgb order is bgr

void bilateralKernel(IplImage *image);
void init_charMat(unsigned char *mat, int n);
void init_floatMat(float *, int n);
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
	//CvCapture *capture = cvCaptureFromCAM(0);

	if(!img)
	{
		printf("Could not load image file: %s\n", argv[1]);
		exit(0);
	}

	/*if(!cvGrabFrame(capture))
	{
		printf("Could not capture device\n");
		exit(0);
	}*/

	//img = cvRetrieveFrame(capture,1);
	
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

	//pca(img);
	//Display the image on the window
	cvShowImage(window, img);

	cvSaveImage("object-detection-output.jpg", img, 0);

	//Wait key to signal exit  
	cvWaitKey(0);

	//Releases the image
	cvReleaseImage(&img);
	//cvReleaseCapture(&capture);
	return 0;
}

void steeringKernel(IplImage *image)
{
	int i, j, k, ii, jj;
	int width, height, step, channels;
	unsigned char *src;
	unsigned char *dest;

	height   = image->height;
	width    = image->width;
	step     = image->widthStep;
	channels = image->nChannels;
	src      = (unsigned char *)image->imageData;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(ii = max(0,i-1); ii <= min(height-1, i+2); i++)
			{
				for(jj = max(0,j-1); jj <= min(width-1,j+1); jj++)
				{
					
				}
			}
		}
	}
	
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

				for(ii = max(0,i-2); ii <= min(height-1,i+2); ii++)
				{
					cols = 0;
					for(jj = max(0, j-2); jj <= min(width-1, j+2); jj++)
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
				dest[i*step+j*channels+k] = exp(colorSqrd * 0.5) + exp(imageSqrd * 0.5);
				//float bilat = ((colorSqrd - imageSqrd) / (float)nbrs);
				//src[i*step+j*channels+k] = exp(-0.5 * (bilat * bilat));
			}
		}
		//printf("done: %d", i);
	}

	memcpy(src, dest, sizeof(unsigned char) * width * height * channels); 
	
	free(dest);
}

void pca(IplImage *image)
{
	int i, j, k;
	int width, height, step, channels;
	unsigned char *src;
	float *u;
	unsigned char *b;
	unsigned char *g;

	height    = image->height;
	width     = image->width;
	step      = image->widthStep;
	channels  = image->nChannels;
	src       = (unsigned char *)image->imageData;
	
	u = (float *)malloc(sizeof(float) * height * channels);
	b = (unsigned char *)malloc(sizeof(unsigned char) * width * height * channels);
	g = (unsigned char *)malloc(sizeof(unsigned char) * height * channels);
	
	//bg = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
	//bb = (unsigned char *)malloc(sizeof(unsigned char) * width * height);

	init_floatMat(u, height * channels);
	init_charMat(b, height * width * channels);
	//init_mat(bg, height * width);
	//init_mat(bb, height * widht);
	init_charMat(g, height * channels);

	//begin by calculating the empirical mean
	//u[1..height] = 1/n sum(src[i,j])
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				//need to fix floating point arithmetic
				u[i*channels + k] += (float)src[i*step+j*channels+k] / (float)width;
			}
		}
	}

	printf("empirical means working\n");

	//we next calculate the deviation from the mean
	//b = src[i,j] - u;
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				b[i*step+j*channels+k] = src[i*step+j*channels+k] - (int)u[i*channels+k];
			}	
		}
	}

	
	printf("deviation working\n");
	//we now need to find the covariance matrix
	
	//b in opencv matrix form
	CvMat bMat = cvMat(height, width, CV_8UC3, b);
	//CvMat bgMat = cvMat(height, width, CV_8U3, bg);
	//CvMat bbMat = cvMat(height, width, CV_8U3, bb);
	
	CvMat *bMatb = cvCreateMat(height, width, CV_8UC1);
	CvMat *bMatg = cvCreateMat(height, width, CV_8UC1);
	CvMat *bMatr = cvCreateMat(height, width, CV_8UC1);

	cvSplit(&bMat, bMatb, bMatg, bMatr, 0);	
	
	//covariance matrix
	CvMat *cb = cvCreateMat(height, height, CV_32FC1);
	CvMat *cg = cvCreateMat(height, height, CV_32FC1);
	CvMat *cr = cvCreateMat(height, height, CV_32FC1);
	CvMat *c = cvCreateMat(height, height, CV_32FC3);
	
	cvMulTransposed(bMatb, cb, 0, NULL, 1.0/(double)width);
	cvMulTransposed(bMatg, cg, 0, NULL, 1.0/(double)width);
	cvMulTransposed(bMatr, cr, 0, NULL, 1.0/(double)width);

	printf("Covariance Matrix working\n");

	//eigenvector and values
        CvMat *eMatb = cvCreateMat(height, height, CV_32FC1);
	CvMat *lMatb = cvCreateMat(height, 1, CV_32FC1);
        CvMat *eMatr = cvCreateMat(height, height, CV_32FC1);
	CvMat *lMatr = cvCreateMat(height, 1, CV_32FC1);
        CvMat *eMatg = cvCreateMat(height, height, CV_32FC1);
	CvMat *lMatg = cvCreateMat(height, 1, CV_32FC1);
	
	printf("Successfully created eigen matrices\n");	

	//cvEigenVV(cb, eMatb, lMatb, 1e-10, -1, -1);
	//cvEigenVV(cg, eMatg, lMatg, 1e-10, -1, -1);
	//cvEigenVV(cr, eMatr, lMatr, 1e-10, -1, -1);
	
	cvSVD(cb, lMatb, eMatb, NULL, CV_SVD_U_T & CV_SVD_MODIFY_A);
	cvSVD(cg, lMatg, eMatg, NULL, CV_SVD_U_T & CV_SVD_MODIFY_A);
	cvSVD(cr, lMatr, eMatr, NULL, CV_SVD_U_T & CV_SVD_MODIFY_A);
	 

	printf("Eigentvectors and Eigenvalues passes\n");

	unsigned char *lb = lMatb->data.ptr;
	unsigned char *eb = eMatb->data.ptr;
	unsigned char *lr = lMatr->data.ptr;
	unsigned char *er = eMatr->data.ptr;
	unsigned char *lg = lMatg->data.ptr;
	unsigned char *eg = eMatg->data.ptr;
	
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < i+1; j++)
		{
			for(k = 0; k < channels; k++)
			{
				g[i*channels] += lb[i*channels+k];
			}
		}
	}	
	
	printf("Successfully computed cumulative energy\n");

	int L = 0;
	float currVal = 0.0;

	/*for(i = 0; i < height; i++)
	{
		if(currVal >= 0.9)
		{
			L = i;
			break;
		}
	
		currVal = 0.0;

		for(k = 0; k < channels; k++)
		{
			currVal += (float)g[i*channels+k] / (float)g[height - 3 + k];		
		}
	}*/

	L = 2;
	printf("Successfully computed L with value of %d\n", L);

	unsigned char *w;
	
	w = (unsigned char *)malloc(sizeof(unsigned char) * height * L * channels);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < L; j++)
		{
			//for(k = 0; k < channels; k++)
			//{	
				w[i*(L*channels)+j*channels]   = eb[i*height+j];
				w[i*(L*channels)+j*channels+1] = eg[i*height+j];
				w[i*(L*channels)+j*channels+2] = er[i*height+j];	
			//}
		}
	}

	printf("Successfully created basis vectors\n");

	unsigned char *s;
	s = (unsigned char *)malloc(sizeof(unsigned char) * height * channels);

	for(i = 0; i < height; i++)
	{
		s[i*channels] = sqrt(cb->data.ptr[i*cb->cols+i]);
		s[i*channels+1] = sqrt(cg->data.ptr[i*cg->cols+i]);
		s[i*channels+2] = sqrt(cr->data.ptr[i*cr->cols+i]);
	}

	printf("Successfully convertd source data to z-scores\n");

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

	printf("Successfully calculated Z\n");

	//Projecting the z-scores of the data onto the new basis
	//CvMat wMatb = cvMat(height, L, CV_32FC1, eb);
	//CvMat wMatr = cvMat(height, L, CV_32FC1, er);
	//CvMat wMatg = cvMat(height, L, CV_32FC1, eg);
	
	CvMat wMat = cvMat(L, height, CV_32FC3, w);

	//cvMerge(&wMatb, &wMatr, &wMatg, 0, wMat);

	CvMat *wMatT = cvCreateMat(height, L, CV_32FC3); 

	cvTranspose(&wMat, wMatT);

	//char *dat = wMatT->data.ptr;

	/*for(i = 0; i < L; i++)
	{
		for(j = 0; j < height; j++)
		{
			for(k = 0; k < channels; k++)
			{
				printf("%d ", dat[i*L+j*channels+k]);
			}
			printf("\n");
		}
	}*/
	
	//CvMat *wMatTb = cvCreateMat(height, height, CV_8UC1);
	//CvMat *wMatTg = cvCreateMat(height, height, CV_8UC1);
	//CvMat *wMatTr = cvCreateMat(height, height, CV_8UC1);

	//cvSplit(wMatT, wMatTb, wMatTg, wMatTr, 0);

	printf("Transpose of W\n");

	CvMat zMat = cvMat(height, width, CV_32FC3, z);

	//CvMat *zMatb = cvCreateMat(height, width, CV_8UC1);
	//CvMat *zMatg = cvCreateMat(height, width, CV_8UC1);
	//CvMat *zMatr = cvCreateMat(height, width, CV_8UC1);

	//cvSplit(&zMat, zMatb, zMatg, zMatr, 0);

	printf("created z matrix\n");
	
	CvMat *yMat = cvCreateMat(height, width, CV_32FC3);

	//CvMat *yMatb = cvCreateMat(height, width, CV_8UC1);
	//CvMat *yMatg = cvCreateMat(height, width, CV_8UC1);
	//CvMat *yMatr = cvCreateMat(height, width, CV_8UC1);
	
	printf("computed y matrix\n");

	char *wdat = wMatT->data.ptr;
	char *zdat = zMat.data.ptr;
	char *ydat = (char *)malloc(sizeof(char) * L * width * channels);
	init_charMat(ydat, L*width*channels);
	int r = 0;

	for(i = 0; i < L; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				for(r = 0; r < width; r++)
				{
					ydat[i*step+j*channels+k] += wdat[i*step+r*channels+k] * zdat[r*step+j*channels+k]; 
				}				
			}
		}
	}

	//char *fnorm = (char *)malloc(sizeof(char) * width);

	 
	/*for(i = 0; i < width * channels; i++)
	{
		printf("%d\n", ydat[i]);
	}*/
	/*float adotb = cvDotProduct(wMatT, &zMat);
	float bdota = cvDotProduct(&zMat, wMatT);

	float div = adotb/bdota;

	

	cvDiv(NULL, &zMat, yMat, div);*/
	//cvMul(wMatT, &zMat, yMat, 1.0);
	//cvMul(wMatTg, zMatg, yMatg, 1.0);
	//cvMul(wMatTr, zMatr, yMatr, 1.0);
        printf("Matrix Multiply Successful\n");
	
	//char *output = yMat->data.ptr;
	
	//printf("height: %d width: %d channels: %d", height, width, channels);
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			for(k = 0; k < channels; k++)
			{
				src[i*step+j*channels+k] = src[i*step+j*channels+k] * ydat[(i*channels)+width+k];
			}
		}
	}
	printf("Successfully multiplied\n");
 	
	
	//cvMerge(yMatb, yMatg, yMatr, 0, yMat);
}

void init_charMat(unsigned char *mat, int n)
{
	int i;

	for(i = 0; i < n; i++)
	{
		mat[i] = 0;
	}
}

void init_floatMat(float *mat, int n)
{
	int i;

	for(i = 0; i < n; i++)
	{
		mat[i] = 0.0;
	}
}



