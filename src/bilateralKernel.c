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
