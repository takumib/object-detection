#ifndef _MATRIX_H_
#define _MATRIX_H_

void empirical_mean(float *src, float *mean, int width, int height);
void deviation(float *src, float *mean, float *dev, int width, int height);
void covariance_matrix(float *dev, float *covar, int width, int height);
void eigen_vv(float *covar, float *eigenVec, float *eigenVal, int height);
void basis_vec(float *eigenVec, float *basis, int height, int d);
void zscore(float *covar, float *deviation, float *z, int width, int height);
void project_zscore(float *basis, float *z, float *p, int width, int height, int d);
void init_charMat(unsigned char *mat, int n);
void init_floatMat(float *mat, int n);

#endif
