extern CvScalar cosine_similarity(CvMat * test, CvMat * patch);
extern double Resemblance(CvScalar map);

#define TOLERANCE 0.96

typedef struct{
	int size;
	double * resemblences;
}resembleList;



