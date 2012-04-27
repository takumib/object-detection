#include "object-detection.h"

// compiled with
//gcc example.c -o example `pkg-config --cflags --libs opencv`

//rgb order is bgr

int main(int argc, char *argv[])
{
	IplImage *src;
	int height, width, step, channels;
	uchar *data;
	char *window;
	float *features, *normalized;
	int d;

	window = "Object Detection";
	src    = 0;

	//If we do not have an input image
	if(argc < 2)
	{
		printf("Usage: object-detection <image-file>\n");
		exit(0);
	}

	//Load image from input
	src =  cvLoadImage(argv[1], LOAD_GRAY);
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
	
	//Information about the image
	printf("Processing a %dx%d image with %d channels\n", height, width, channels); 

	//Set up basic window
	cvNamedWindow(window, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(window, 100, 100);

	steering_kernel(src);

	features = pca(src, &d);
	
	normalized = (float *)malloc(sizeof(float) * width * d);

	normalize_features(features, normalized, width, d);

	//Display the image on the window
	cvShowImage(window, src);

	//cvSaveImage("object-detection-output.jpg", src, 0);

	//Wait key to signal exit  
	cvWaitKey(0);

	//Releases the image
	cvReleaseImage(&src);
	//cvReleaseCapture(&capture);

	return 0;
}

void steering_kernel(IplImage *image)
{
	int i, j, k, ii, jj;
	int width, height, step, channels;
	uchar *src;
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
    int count 		  = 0;

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
			float dist    = exp(-1 * sqrt(output[0] * output[0] +  output[1] * output[1])  / (2 *  0.008));
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

}


float *pca(IplImage *image, int *energy)
{
	int i, j;
	int d;
	int width, height, step, channels;
	uchar *src;
	float *data;
	float *mean, *dev, *covar, *eigenVec, *eigenVal, *basis, *z, *p;
	
	height    = image->height;
	width     = image->width;
	src       = (unsigned char *)image->imageData;
	
	d         = 2;
	mean      = (float *)malloc(sizeof(float) * height);
	dev       = (float *)malloc(sizeof(float) * width * height);
	covar     = (float *)malloc(sizeof(float) * height * height);
	eigenVec  = (float *)malloc(sizeof(float) * height * height);
	eigenVal  = (float *)malloc(sizeof(float) * height);
	basis     = (float *)malloc(sizeof(float) * d * height);
	z         = (float *)malloc(sizeof(float) * width * height);
	p         = (float *)malloc(sizeof(float) * width * d);
	
	init_floatMat(mean, height);
	init_floatMat(dev, width * height);
	init_floatMat(covar, height * height);
	init_floatMat(eigenVec, height * height);
	init_floatMat(eigenVal, height);
	init_floatMat(basis, d * height);
	init_floatMat(z, width * height);
	init_floatMat(p, width * d);	

	data = (float *)malloc(sizeof(float) * width * height);

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			data[i * width + j] = (float)src[i * width + j];
		}
	}
	
	//printf("calculating empircal mean ...\n");
	//calculate empirical mean
	empirical_mean(data, mean, width, height);
	//printf("empirical means working\n");

	//printf("calculating deviation from mean ...\n");
	//calculate deviation from the mean
	deviation(data, mean, dev, width, height);
	//printf("deviation working\n");
	
	//printf("calculating covariance matrix ...\n");
	//covariance matrix
	covariance_matrix(dev, covar, width, height);
	//printf("covariance matrix working ...\n");

	//printf("calculating eigen ...\n");
	eigen_vv(covar, eigenVec, eigenVal, height);
	//printf("eigen working ...\n");

	//printf("calculating basis ...\n");
	basis_vec(eigenVec, basis, height, d);
	//printf("basis working ...\n");

	//printf("calculating zscore ...\n");
	zscore(covar, dev, z, width, height);
	//printf("zscore working ...\n");

	//printf("calculating project_zscore ...\n");
	project_zscore(basis, z, p, width, height, d);
	//printf("project_zscore working ...\n");
 	
	memcpy(energy, &d, sizeof(int));

	return p;
}

void normalize_features(float *features, float *normalized, int width, int d)
{
	int i, j;
	float vecNorm;

	vecNorm = 0.0;

	for(i = 0; i < width; i++)
	{
		for(j = 0; j < d; j++)
		{
			float feature = features[i + (j * width)];

			vecNorm += feature * feature;
		}
	}

	vecNorm = sqrt(vecNorm);

	for(i = 0; i < d; i++)
	{
		for(j = 0; j < width; j++)
		{
			normalized[i * width + j] = features[i * width + j] / vecNorm;
		}
	}
}


