#include "stdafx.h"
#include "Main.h"

class IPMAlg{
public:
	IPMAlg(Mat src, vector<Point> roiPoints);

	Mat remapImage(Mat src);
	Mat remapInvImage(Mat src);
private:
	double width, height;
	Mat perspectiveTransformMat, inversePerspectiveTransformMat;
	Mat mapX, mapY, invMapX, invMapY;

	void createMaps();
	Point2f computePoint(Point point, Mat mat);
};