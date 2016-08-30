#include "stdafx.h"
#include "Main.h"

class ArrowClassification {
public:
	enum ArrowType { RIGHT, LEFT, UP, DOWN };
	ArrowClassification(Mat grayImage, Mat colorImage);
	void harris();
private:
	Mat src;
	Mat colorImageToDraw;
	double height, width;

	void cornersClasification(vector<pair<Point, int>> cornersWithLabels);
	void classification(vector<Point> corners);
	ArrowType findOrientation(vector<Point> corners, vector<int> indexCollinearPoints);
	void drawRectangle(vector<Point> corners, ArrowType arrowType);
	bool isLeft(Point a, Point b, Point c);
	bool isUp(Point a, Point b, Point c);
	bool contains(vector<int> vector, int x);
};