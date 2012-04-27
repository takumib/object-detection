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

