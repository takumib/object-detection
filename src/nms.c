#include <stdlib.h>
#include <cv.h>
#include <stdio.h>
#include "cosinesimilarity.h"
#define DEBUG 1


resembleList NMS(resembleList list)
{
	//Does the non-maximum suppression 
	// list has already been filtered for tolerances

	int i,j,k,l, temp_i, temp_j;
	double temp;


	//For each spot in the resemblence map
	for (i = 0; i < res.size;i++)
	{

		for (j = 0; j < res.size;i++)
		{
			//If there is  a response
			// If any index is greater than 0, it is a repsonse
			if (list.resemblences[i][j] > 0.0)
			{
				//now that we have a reponse, we store it 
				temp = list.resemblences[i][j];

				//and its location
				temp_i = i;
				temp_j = j;

				//From the top to the bottom
				for(k = i - res.width; (k < i+ list.width); k++)
				{
					//From the left to the right 
					for(l = j - res.width; l < (j + list.width);l++)
					{
						//making sure not to go out of the bounds of the array
						if((k < (res.size)) && (k > -1) &&(j<res.size)&&(j>-1))
						{

							//if the resemblence is bigger than our current champ							
							if(list.resemblences[i][j] > temp)
							{
								//Then replace it and store new index
								temp = list.resemblences[i][j];
								temp_i = i;
								temp_j = j;			
							}else{
								//if not, zero the weakling out				
								list.resemblences[i][j] = 0.0;
							}
						}

					}
				}
			}
		}
	}
}
