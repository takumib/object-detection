#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

#define LOAD_RGB  CV_LOAD_IMAGE_COLOR
#define LOAD_GRAY CV_LOAD_IMAGE_GRAYSCALE

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)<(b))?(b):(a))

typedef unsigned char uchar;

#endif
