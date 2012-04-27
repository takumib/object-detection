#ifndef _OBJECT_DETECTION_H
#define _OBJECT_DETECTION_H

#include "definitions.h"
#include "steeringKernel.h"
#include "matrix.h"
#include "cosinesimilarity.h"

void bilateralKernel(IplImage *image);
void init_charMat(unsigned char *mat, int n);
void init_floatMat(float *, int n);
void steering_kernel(IplImage *image, float *weights);
float *pca(IplImage *image, int *energy);
void normalize_features(float *features, float *normalized, int width, int d);

#endif
