#include "stdafx.h"
#include "Main.h"

class ROI {
public:
	vector<Point> roiPoints;

	ROI(Mat image_uchar);
	void drawROI(Mat image);
	void ROI::findLineOnROI(Mat image);
private:
	Mat src;
	vector<Vec2f> houghLines;
	Vec2f leftExteriorLineCoord, rightExteriorLineCoord;
	double width, height;

	void callHough();
	void findExteriorLineMarkings();
	boolean isInLeftInterval(double theta);
	boolean isInRightInterval(double theta);
	void drawSelectedLines(Mat image, double theta, double rho);
	vector<Point> getROI();
	vector<Point> findCurrentLane();
	vector<Point> findPointsForCoord(Vec2f coord);
	Point findIntersectionPoint(Point p1Line1, Point p2Line1, Point p1Line2, Point p2Line2);
	vector<double> computeLineParam(Point p1, Point p2);
};