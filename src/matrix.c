#include "definitions.h"

/**
 * emperical_mean:
 * Calculates the empirical mean of an mxn matrix.
 * The emperical mean is the sum of the data in the columns
 * divided by the number of columns.
 * dest[i] = 1/n * sum(src[i,j], n = 1..n), i = 1..m.
 * 
 * @param src an mxn matrix that stores our intitial data
 * @param mean an mx1 matrix our emperical mean matrix
 * @param width the number of columns in our matrix (n)
 * @param height the number of rows in our matrix (m)
 *
 * @return mean our empirical mean
 */
void empirical_mean(float *src, float *mean, int width, int height)
{
	int i, j;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
				mean[i] += (float)src[i * width + j] / (float)width; 
		}
	}
}

/**
 * deviation: 
 * Calculates the deviation from the mean of an mxn matrix.
 * The deviation from the mean is the difference from the
 * src at i,j from the mean at i.
 * dest[i][j] = src[i][j] - mean[i], i = 1..m, j = 1..n.
 * 
 * @param src an mxn matrix that stores our initial dataset
 * @param mean an mx1 matrix that stores our emperical mean
 * @param dev an mxn matrix that will store our deviation
 * @param width the number of columns in our matrix (n)
 * @param height the number of rows in our matrix (m)
 *
 * @return dev our deviation from mean matrix
 */
void deviation(float *src, float *mean, float *dev, int width, int height)
{
	int i, j;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{			
			dev[i * width + j] = src[i * width + j] -  mean[i]; 
		}
	}	
}

/**
 * covariance_matrix: 
 * This finds the emperical covariance matrix from the deviation.
 * The covariance matrix is the product of our deviation matrix
 * multiplied the transpose of the deviation matrix, divided by the
 * number of columns.
 * dest = 1/n * (dest * destT).
 * 
 * @param dev an mxn matrix that is our deviation from the mean
 * @param covar an mxm matrix that will store our covariance matrix
 * @param width the number of columns in our matrix (n)
 * @param height the number of rows in our matrix (m)
 *
 * @return covar our covariance matrix
 */
void covariance_matrix(float *dev, float *covar, int width, int height)
{
	CvMat devMat    = cvMat(height, width, CV_32FC1, dev);
	CvMat *covarMat = cvCreateMat(height, height, CV_32FC1);

	cvMulTransposed(&devMat, covarMat, 0, NULL, 1.0 / (float)width);

	memcpy(covar, covarMat->data.fl, sizeof(float) * height * height);
}

/**
 * eigen_vv: 
 * Calculates the singular value decomposition for the covariance matrix.
 * Important for calculating the eigenvectors and eigenvalues.
 * 
 * @param covar an mxm matrix that is our covariance matrix
 * @param eigenVec mxm matrix of eigenvectors
 * @param eigenVal mx1 matrix of eigenvalues
 * @param height the number of rows in our matrix (m)
 *
 * @return eigenVec creates our eigenvectors from SVD
 * @return eigenVal creates our eigenvalues from SVD
 */
void eigen_vv(float *covar, float *eigenVec, float *eigenVal, int height)
{
	CvMat covarMat = cvMat(height, height, CV_32FC1, covar);
	CvMat *eigenVecMat = cvCreateMat(height, height, CV_32FC1);
	CvMat *eigenValMat = cvCreateMat(height, 1, CV_32FC1);

	cvSVD(&covarMat, eigenVecMat, eigenValMat, NULL, CV_SVD_U_T);

	memcpy(eigenVec, eigenVecMat->data.fl, sizeof(float) * height * height);
	memcpy(eigenVal, eigenValMat->data.fl, sizeof(float) * height);
}

/**
 * basis_vec: 
 * Gets the basis vectors for highest energies.
 * basis[i][j] = eigenVec[i][j], i = 1..m, j = 1..d.
 * 
 * @param eigenVec an mxm matrix that is our eigenvectors
 * @param basis an mxd matrix that will store our basis vectors
 * @param height the number of rows in our matrix (m)
 * @param d our highest energy values
 * 
 * @return basis creates our basis vectors
 */
void basis_vec(float *eigenVec, float *basis, int height, int d)
{
	int i, j;

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < d; j++)
		{
			basis[i * d + j] = eigenVec[i * height + j];
		}
	}
}

/**
 * zscore: 
 * Computes the standard deviation matrix
 * z = deviation / s, s = standard deviation.
 * 
 * @param covar an mxm matrix that stores our covariance matrix
 * @param deviation an mxn matrix that stores our deviation
 * @param z an mxn matrix that will store our zscore matrix
 * @param width the number of columns in our matrix (n)
 * @param height the number of rows in our matrix (m)
 *
 * @return z modifies the z matrix
 */
void zscore(float *covar, float *deviation, float *z, int width, int height)
{
	int i, j;
	float *s;

	s = (float *)malloc(sizeof(float) * height);

	for(i = 0; i < height; i++)
	{
		s[i] = sqrt(covar[i * height + i]);
	}

	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			z[i * width + j] = deviation[i * width + j] / s[i];
		}
	}
	
	free(s);
}

/**
 * project_zscore: 
 * projection of our zscore with our basis vectors.
 * p = basisT * z, (basis transpose)
 * 
 * @param basis an mxd matrix that stores our basis vectors
 * @param z an mxn matrix that stores our z score matrix
 * @param p an dxn matrix that will store our projection
 * @param width the number of columns in our matrix (n)
 * @param height the number of rows in our matrix (m)
 * @param d our highest energy values
 *
 * @return p modifies the projected matrix
 */
void project_zscore(float *basis, float *z, float *p, int width, int height, int d)
{
	int i, j, k;

	CvMat basisMat  = cvMat(height, d, CV_32FC1, basis);
	CvMat *basisMatT = cvCreateMat(d, height, CV_32FC1);

	cvTranspose(&basisMat, basisMatT);
	
	float *basisT = basisMatT->data.fl;

	for(i = 0; i < d; i++)
	{
		for(j = 0; j < height; j++)
		{
			for(k = 0; k < width; k++)
			{
				p[i * width + j] += basisT[i * height + k] * z[k * width + j]; 
			}				
		}
	}

}

/**
 * init_charMat: 
 * initializes a one dimensional character array of size n to 0
 * 
 * @param mat an n sized char matrix
 * @param n number of elements
 *
 * @return mat modifies the matrix to set all elements to 0
 */
void init_charMat(unsigned char *mat, int n)
{
	int i;

	for(i = 0; i < n; i++)
	{
		mat[i] = 0;
	}
}

/**
 * init_floatMat: 
 * initializes a one dimensional float array to 0.0
 * 
 * @param mat an n sized float matrix
 * @param n number of elements
 *
 * @return mat modifies the matrix to set all elements to 0.0
 */
void init_floatMat(float *mat, int n)
{
	int i;

	for(i = 0; i < n; i++)
	{
		mat[i] = 0.0;
	}
}

