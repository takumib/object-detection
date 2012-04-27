#ifndef _STEERING_KERNEL_H_
#define _STEERING_KERNEL_H_

void compute_xgrad(unsigned char *image, unsigned char *x_component, int width, int height);
void compute_ygrad(unsigned char *image, unsigned char *y_component, int width, int height);

#endif
