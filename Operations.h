#include "stdafx.h"
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

bool isInRange(Mat src, int i, int j);
Mat dilation(Mat src, vector<int> convMatrixI, vector<int> convMatrixJ);
Mat erosion(Mat src, vector<int> convMatrixI, vector<int> convMatrixJ);
Mat opening(Mat src, const int convolutionSize);
Mat closing(Mat src, const int convolutionSize);
Mat etichetare(Mat src, Mat opened);
Mat binarization(Mat src);
Mat makeImageColor(Mat src);
Mat otherMarkingsElimination(Mat src, int threshold, vector<Point>& markingPoints, int minLineWidth);
int transversalMarking(Mat src, int threshold, int minLineWidth);
bool checkMarking(vector<Point> markingPoints, int maxWidth);
Mat intToUcharMat(Mat src);
Mat ucharToIntMat(Mat src);
Mat ucharToLongMat(Mat src);
Mat longToUcharMat(Mat src);