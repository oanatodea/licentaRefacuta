#include "stdafx.h"
#include "IPMAlg.h"

IPMAlg::IPMAlg(Mat src, vector<Point> roiPoints) {
	width = src.cols;
	height = src.rows;
	vector<Point2f> srcPoints{ roiPoints[0], roiPoints[1], roiPoints[2], roiPoints[3] };
	vector<Point2f> dstPoints{ Point2f(0, height), Point2f(0, 0), Point2f(width, 0), Point2f(width, height) };

	perspectiveTransformMat = getPerspectiveTransform(srcPoints, dstPoints);
	inversePerspectiveTransformMat = perspectiveTransformMat.inv();
	createMaps();
}

void IPMAlg::createMaps() {
	mapX.create(height, width, CV_32F);
	mapY.create(height, width, CV_32F);
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			Point2f pt = computePoint(Point(j, i), inversePerspectiveTransformMat);
			mapX.at<float>(i,j) = pt.x;
			mapY.at<float>(i, j) = pt.y;
		}
	}

	invMapX.create(height, width, CV_32F);
	invMapY.create(height, width, CV_32F);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			Point2f pt = computePoint(Point(j, i), perspectiveTransformMat);
			invMapX.at<float>(i, j) = pt.x;
			invMapY.at<float>(i, j) = pt.y;
		}
	}
}

Point2f IPMAlg::computePoint(Point point, Mat mat) {
	Point2f computedPoint = Point2f(-1, -1);
	double x = point.x, y = point.y;
	double u = mat.at<double>(0, 0) * x + mat.at<double>(0, 1) * y + mat.at<double>(0, 2);
	double v = mat.at<double>(1, 0) * x + mat.at<double>(1, 1) * y + mat.at<double>(1, 2);
	double s = mat.at<double>(2, 0) * x + mat.at<double>(2, 1) * y + mat.at<double>(2, 2);
	if (s != 0)
	{
		computedPoint.x = (u / s);
		computedPoint.y = (v / s);
	}
	return computedPoint;
}

Mat IPMAlg::remapImage(Mat src) {
	Mat dst;
	remap(src, dst, mapX, mapY, INTER_LINEAR, BORDER_CONSTANT);
	return dst;
}

Mat IPMAlg::remapInvImage(Mat src) {
	Mat dst;
	remap(src, dst, invMapX, invMapY, INTER_LINEAR, BORDER_CONSTANT);
	return dst;
}