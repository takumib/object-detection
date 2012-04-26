#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h> 
#include "window.h"
#include "cosinesimilarity.h"

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
void steeringKernel(IplImage *image);
void compute_xgrad(unsigned char *image, unsigned char *x_component, int width, int height);
void compute_ygrad(unsigned char *image, unsigned char *y_component, int width, int height);

/*
The process:
test = whole image
query = image with the template

Do PCA on the whole test image 
For each patch do the cosine similarity
Generate a resemblance percentage
*/

int main(int argc, char *argv[])
{
	int height, width, step, channels;
	unsigned char *data;
	char *window = "Object Detection";
	int i, j, k;
	CvMat stub,* query_mat,* test_mat;


	//If we do not have an input image
	if(argc < 3)
	{
		printf("Usage: object-detection <image> <query image>\n");
		exit(0);
	}

	//Load image from input
	IplImage *img = NULL;
	IplImage *src = NULL;
	IplImage *query = NULL;
	src =  cvLoadImage(argv[1], LOAD_RGB);
	query = cvLoadImage(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
	//CvCapture *capture = cvCaptureFromCAM(0);

	if(!src)
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
	height    = src->height;
	width     = src->width;
	step      = src->widthStep;
	channels  = src->nChannels;
	data      = (unsigned char *)src->imageData;


	img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	cvCvtColor(src, img, CV_BGR2GRAY);

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

	//unsigned char *test = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
	IplImage *b = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	IplImage *g = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	IplImage *r = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	cvSplit(src, b, g, r, 0);	
	steeringKernel(b);
	steeringKernel(g);
	steeringKernel(r);

	pca(b);
	pca(g);
	pca(r);

	cvMerge(b, g, r, 0, src);

	//steeringKernel(img);

	//memcpy(data, test, sizeof(unsigned char) * width * height);
	//Invert the image
	//bilateralKernel(src);

	//pca(src);
	//Display the image on the windowi
	cvShowImage(window, src);
	//cvShowImage("b", b);
	//cvShowImage("g", g);

	query_mat = cvGetMat(query, &stub,0,0);

	printf("starting the patching process\n");

	//get the patches
	CvMat ** patches = get_queries(src, query);
	printf("Patches done, %d of them %d wide\n",(src->height - (query->height-1)), patches[0]->rows );

	double ** resemblence_map = malloc(src->height * src->width * sizeof(double *));

	for(i = 0; i < src->height; i++){
		resemblence_map[i] = malloc(src->height * sizeof(double));
	} 


	for(i = 0; i < (src->height - (query->height-1));i++){
		printf("Doing similarity..");
		CvScalar similarity = cosine_similarity(patches[i],query_mat);
		printf("... finished\n");	
		resemblence_map[i][j % src->height] = Resemblance(similarity);
	}


	cvSaveImage("object-detection-output.jpg", src, 0);

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
	float *dest;

	height   = image->height;
	width    = image->width;
	step     = image->widthStep;
	channels = image->nChannels;
	src      = (unsigned char *)image->imageData;

	char *gx = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
	char *gy = (unsigned char *)malloc(sizeof(unsigned char) * width * height);

	compute_xgrad(src, gx, width, height);
	compute_ygrad(src, gy, width, height);

	CvMat *covarMat   = cvCreateMat(2, 2, CV_32FC1);
	CvMat *spatialMat = cvCreateMat(1, 2, CV_32FC1); 
	CvMat *outputMat  = cvCreateMat(1, 2, CV_32FC1);

	float *covar      = covarMat->data.fl;
	float *spatial    = spatialMat->data.fl;
	int count = 0;

	dest = (float *)malloc(sizeof(float) * width * height);

	int xg[] = {-1, 0, 1,
		-2, 0, 2,
		-1, 0, 1};

	int yg[] = {-1, -2, -1,
		0,  0,  0,
		1,  2,  1};

	for(i = 1; i < height - 1; i++)
	{
		for(j = 1; j < width - 1; j++)
		{
			count = 0;
			cvmSet(spatialMat, 0, 0, 0);
			cvmSet(spatialMat, 0, 1, 0);

			for(ii = max(0,i-1); ii <= min(height-1, i+1); ii++)
			{
				for(jj = max(0,j-1); jj <= min(width-1,j+1); jj++)
				{
					cvmSet(spatialMat, 0, 0, cvmGet(spatialMat, 0, 0) + ((jj - j) * xg[count]));//cvmGet(spatialMat, 0, 0) + (jj - j));
					cvmSet(spatialMat, 0, 1, cvmGet(spatialMat, 0, 1) + ((ii - i) * yg[count]));//cvmGet(spatialMat, 0, 1) + (ii - i));
					count ++;
					//spatial[0] += jj - j;
					//spatial[1] += ii - i;					
				}
			}

			//printf("count: %d\n", count);
			cvmSet(covarMat, 0, 0, gx[i * width + j] * gx[i * width + j]);
			cvmSet(covarMat, 0, 1, gx[i * width + j] * gy[i * width + j]);
			cvmSet(covarMat, 1, 0, gx[i * width + j] * gy[i * width + j]);
			cvmSet(covarMat, 1, 1, gy[i * width + j] * gy[i * width + j]);

			cvMatMul(spatialMat, covarMat, outputMat);
			float *output = outputMat->data.fl;
			float dist = exp(-1 * sqrt(output[0] * output[0] +  output[1] * output[1])  / (2 *  0.008));
			//double det = cvDet(covarMat);

			//printf("covar: %f\n",  det);

			dest[i * width + j] = dist; 		
		}
	}

	//computing weights

	float kernelSum;

	for(i = 1; i < height - 1; i++)
	{
		for(j = 1; j < width - 1; j++)
		{
			kernelSum = 0;

			for(ii = max(0,i-1); ii <= min(height-1, i+1); ii++)
			{
				for(jj = max(0,j-1); jj <= min(width-1,j+1); jj++)
				{
					//printf("%f\n", dest[ii * width + jj]);
					kernelSum += dest[ii * width + jj];					
				}
			}
			src[i * width + j] =  (unsigned char)(src[i * width + j] * (1 + (dest[i * width + j] / kernelSum)));
			//printf("weighted: %f\n", dest[i * width + j] / kernelSum);	
		}
	}

	//memcpy(dest, src, sizeof(unsigned char) * width * height);

}

void compute_xgrad(unsigned char *image, unsigned char *x_component, int width, int height)
{
	int i, j;

	int gradSum;

	//x gradient sobel operator
	/*{-1, 0, 1,
	  -2, 0, 2,
	  -1, 0, 1,};*/ 

	for(i = 1; i < height - 1; i++)
	{
		for(j = 1; j < width - 1; j++)
		{
			gradSum = (-1 * image[(i - 1) * width + (j - 1)]) + 
				(image[(i - 1) * width + (j + 1)]) + 
				(-2 * image[i * width + (j - 1)]) + 
				(2 * image[i * width + (j + 1)]) +
				(-1 * image[(i + 1) * width + (j - 1)]) +
				(image[(i + 1) * width + (j + 1)]);
			x_component[i * width + j] = gradSum;			
		}
	}

}

void compute_ygrad(unsigned char *image, unsigned char *y_component, int width, int height)
{
	int i, j;
	int gradSum;

	//y gradient sobel operator
	/* {-1 -2, -1,
	   0, 0, 0,
	   1, 2, 1}*/

	for(i = 1; i < height - 1; i++)
	{
		for(j = 1; j < width - 1; j++)
		{
			gradSum = (-1 * image[(i - 1) * width + (j - 1)]) +
				(-2 * image[(i - 1) * width + j]) + 
				(-1 * image[(i - 1) * width + (j + 1)]) + 
				(image[(i + 1) * width + (j - 1)]) + 
				(2 * image[i * width + j]) +
				(image[(i + 1) * width + (j + 1)]);

			y_component[i * width + j] = gradSum;			
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

	u = (float *)malloc(sizeof(float) * height);
	b = (unsigned char *)malloc(sizeof(unsigned char) * width * height);
	g = (unsigned char *)malloc(sizeof(unsigned char) * height);	

	init_floatMat(u, height);
	init_charMat(b, height * width);
	init_charMat(g, height);

	//begin by calculating the empirical mean
	//u[1..height] = 1/n sum(src[i,j])
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			//need to fix floating point arithmetic
			u[i] += (float)src[i*step+j] / (float)width;
		}
	}

	printf("empirical means working\n");

	//we next calculate the deviation from the mean
	//b = src[i,j] - u;
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			b[i*step+j] = src[i*step+j] - (int)u[i];
		}
	}


	printf("deviation working\n");
	//we now need to find the covariance matrix

	//b in opencv matrix form
	CvMat bMat = cvMat(height, width, CV_8UC1, b);
	//CvMat bgMat = cvMat(height, width, CV_8U3, bg);
	//CvMat bbMat = cvMat(height, width, CV_8U3, bb);

	//CvMat *bMatb = cvCreateMat(height, width, CV_8UC1);
	//CvMat *bMatg = cvCreateMat(height, width, CV_8UC1);
	//CvMat *bMatr = cvCreateMat(height, width, CV_8UC1);

	//cvSplit(&bMat, bMatb, bMatg, bMatr, 0);	

	//covariance matrix
	//CvMat *cb = cvCreateMat(height, height, CV_32FC1);
	//CvMat *cg = cvCreateMat(height, height, CV_32FC1);
	//CvMat *cr = cvCreateMat(height, height, CV_32FC1);
	CvMat *c = cvCreateMat(height, height, CV_32FC1);

	//cvMulTransposed(bMatb, cb, 0, NULL, 1.0/(double)width);
	//cvMulTransposed(bMatg, cg, 0, NULL, 1.0/(double)width);
	//cvMulTransposed(bMatr, cr, 0, NULL, 1.0/(double)width);

	cvMulTransposed(&bMat, c, 0, NULL, 1.0/(double)width);

	printf("Covariance Matrix working\n");

	//eigenvector and values
	//     CvMat *eMatb = cvCreateMat(height, height, CV_32FC1);
	//CvMat *lMatb = cvCreateMat(height, 1, CV_32FC1);
	//   CvMat *eMatr = cvCreateMat(height, height, CV_32FC1);
	//CvMat *lMatr = cvCreateMat(height, 1, CV_32FC1);
	//  CvMat *eMatg = cvCreateMat(height, height, CV_32FC1);
	//CvMat *lMatg = cvCreateMat(height, 1, CV_32FC1);

	CvMat *eMat = cvCreateMat(height, height, CV_32FC1);
	CvMat *lMat = cvCreateMat(height, 1, CV_32FC1);

	printf("Successfully created eigen matrices\n");	

	//cvEigenVV(cb, eMatb, lMatb, 1e-10, -1, -1);
	//cvEigenVV(cg, eMatg, lMatg, 1e-10, -1, -1);
	//cvEigenVV(cr, eMatr, lMatr, 1e-10, -1, -1);

	//cvSVD(cb, lMatb, eMatb, NULL, CV_SVD_U_T & CV_SVD_MODIFY_A);
	//cvSVD(cg, lMatg, eMatg, NULL, CV_SVD_U_T & CV_SVD_MODIFY_A);
	//cvSVD(cr, lMatr, eMatr, NULL, CV_SVD_U_T & CV_SVD_MODIFY_A);

	cvSVD(c, lMat, eMat, NULL, CV_SVD_U_T); 

	printf("Eigentvectors and Eigenvalues passes\n");

	//unsigned char *lb = lMatb->data.ptr;
	//unsigned char *eb = eMatb->data.ptr;
	//unsigned char *lr = lMatr->data.ptr;
	//unsigned char *er = eMatr->data.ptr;
	//unsigned char *lg = lMatg->data.ptr;
	//unsigned char *eg = eMatg->data.ptr;

	char *e = eMat->data.ptr;
	char *l = lMat->data.ptr;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < i+1; j++)
		{
			g[i] += l[i];
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

	w = (unsigned char *)malloc(sizeof(unsigned char) * height * L);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < L; j++)
		{
			w[i*L+j] = e[i*height+j];
		}
	}

	printf("Successfully created basis vectors\n");

	unsigned char *s;
	s = (unsigned char *)malloc(sizeof(unsigned char) * height);

	for(i = 0; i < height; i++)
	{
		s[i] = sqrt(c->data.ptr[i*c->step+i]);
	}

	printf("Successfully converted source data to z-scores\n");

	unsigned char *z;
	z = (unsigned char *)malloc(sizeof(unsigned char) * height * width);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			z[i*step+j] = (float)b[i*step+j] / (float)s[i];
		}
	}

	printf("Successfully calculated Z\n");

	//Projecting the z-scores of the data onto the new basis
	//CvMat wMatb = cvMat(height, L, CV_32FC1, eb);
	//CvMat wMatr = cvMat(height, L, CV_32FC1, er);
	//CvMat wMatg = cvMat(height, L, CV_32FC1, eg);

	CvMat wMat = cvMat(L, height, CV_32FC1, w);

	//cvMerge(&wMatb, &wMatr, &wMatg, 0, wMat);

	CvMat *wMatT = cvCreateMat(height, L, CV_32FC1); 

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

	CvMat zMat = cvMat(height, width, CV_32FC1, z);

	//CvMat *zMatb = cvCreateMat(height, width, CV_8UC1);
	//CvMat *zMatg = cvCreateMat(height, width, CV_8UC1);
	//CvMat *zMatr = cvCreateMat(height, width, CV_8UC1);

	//cvSplit(&zMat, zMatb, zMatg, zMatr, 0);

	printf("created z matrix\n");

	CvMat *yMat = cvCreateMat(height, width, CV_32FC1);

	//CvMat *yMatb = cvCreateMat(height, width, CV_8UC1);
	//CvMat *yMatg = cvCreateMat(height, width, CV_8UC1);
	//CvMat *yMatr = cvCreateMat(height, width, CV_8UC1);

	printf("computed y matrix\n");

	char *wdat = wMatT->data.ptr;
	char *zdat = zMat.data.ptr;
	char *ydat = (char *)malloc(sizeof(char) * L * width);
	init_charMat(ydat, L*width);
	int r = 0;

	for(i = 0; i < L; i++)
	{
		for(j = 0; j < height; j++)
		{
			for(r = 0; r < width; r++)
			{
				ydat[i*step+j] += wdat[i*step+r] * zdat[r*step+j]; 
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

	//normalize feature vectors
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			src[i*step+j] = src[i*step+j] * ydat[i];
		}
	}
	printf("Successfully normalized\n");


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



