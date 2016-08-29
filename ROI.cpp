#include "stdafx.h"
#include "ROI.h"

ROI::ROI(Mat image_uchar) {
	src = image_uchar;
	width = src.cols;
	height = src.rows;
	callHough();
	findExteriorLineMarkings();
	roiPoints = getROI();
}

void ROI::callHough() {
	//empiric
	HoughLines(src, houghLines, 1, M_PI / 180, 100, 0, 0);
}

void ROI::findExteriorLineMarkings() {
	vector<double> t;
	Mat imageWithHoughLines = src.clone();
	for (int i = 0; i < houghLines.size(); i++)
	{
		double rho = houghLines[i][0], theta = houghLines[i][1];
		if (isInLeftInterval(theta) || isInRightInterval(theta)) {
			drawSelectedLines(imageWithHoughLines, theta, rho);
			if (isInLeftInterval(theta)) {
				if (leftExteriorLineCoord[0] == NULL || rho < leftExteriorLineCoord[0] || (rho == leftExteriorLineCoord[0] && theta < leftExteriorLineCoord[1])) {//theta < leftExteriorLineCoord[1] || (theta == leftExteriorLineCoord[1] && rho < leftExteriorLineCoord[0])) {
					leftExteriorLineCoord = houghLines[i];
				}
			}
			if (isInRightInterval(theta)) {
				if (rightExteriorLineCoord[0] == NULL || theta > rightExteriorLineCoord[1] || (theta == rightExteriorLineCoord[1] && rho < rightExteriorLineCoord[0])) {
					rightExteriorLineCoord = houghLines[i];
				}
			}
		}
	}
	imshow("Hough Lines", imageWithHoughLines);
}

void ROI::drawSelectedLines(Mat image, double theta, double rho) {
	Point pt1, pt2;
	double a = cos(theta), b = sin(theta);
	double x0 = a*rho, y0 = b*rho;
	pt1.x = cvRound(x0 + 1000 * (-b));
	pt1.y = cvRound(y0 + 1000 * (a));
	pt2.x = cvRound(x0 - 1000 * (-b));
	pt2.y = cvRound(y0 - 1000 * (a));
	line(image, pt1, pt2, Scalar(255, 255, 255), 1, LINE_AA);
}

boolean ROI::isInLeftInterval(double theta) {
	double theta_grades = theta * 180 / M_PI;
	if (theta_grades < 60 && theta_grades > 30) {
		return true;
	}
	return false;
}

boolean ROI::isInRightInterval(double theta) {
	double theta_grades = theta * 180 / M_PI;
	if (theta_grades >120 && theta_grades < 150) {
		return true;
	}
	return false;
}

vector<Point> ROI::getROI() {
	vector<Point> lanePoints = findCurrentLane();
	Point pLeft = lanePoints[0], pRight = lanePoints[1], pIntersection = lanePoints[2];
	double yDif;
	if (pLeft.y > height) {
		yDif = abs(height - pIntersection.y);
	}
	else {
		yDif = abs(pLeft.y - pIntersection.y);
	}
	Point p1New, p2New;
	p1New.y = pIntersection.y + yDif / 5;
	p2New.y = pIntersection.y + yDif / 5;
	p1New.x = 0;
	p2New.x = width - 1;
	Point pIntersectionLeft = findIntersectionPoint(pLeft, pIntersection, p1New, p2New);
	Point pIntersectionRight = findIntersectionPoint(pRight, pIntersection, p1New, p2New);
	return vector<Point> { pLeft, pIntersectionLeft, pIntersectionRight, pRight };
}


vector<Point> ROI::findCurrentLane() {
	vector<Point> pointsLeft, pointsRight;
	double diag = sqrt(width * width + height * height);
	if (leftExteriorLineCoord[0] == NULL && rightExteriorLineCoord[0] == NULL) {
		pointsLeft = vector < Point > {Point(0, height), Point(width / 2 - 50, height - 200)};
		pointsRight = vector < Point > {Point(width / 2 + 30, height - 200), Point(width, height)};
	}
	else {
		if (leftExteriorLineCoord[0] == NULL) {
			// only left line not found
			leftExteriorLineCoord[1] = M_PI - rightExteriorLineCoord[1];
			leftExteriorLineCoord[0] = 0;
		}
		else {
			if (rightExteriorLineCoord[0] == NULL) {
				// only right line not found
				rightExteriorLineCoord[1] = M_PI - leftExteriorLineCoord[1];
				rightExteriorLineCoord[0] = 0;
			}
		}
		pointsLeft = findPointsForCoord(leftExteriorLineCoord);
		pointsRight = findPointsForCoord(rightExteriorLineCoord);
	}
	Point intersectionPoint = findIntersectionPoint(pointsLeft[0], pointsLeft[1], pointsRight[0], pointsRight[1]);
	return vector < Point > {pointsLeft[0], pointsRight[1], intersectionPoint};
}

vector<Point> ROI::findPointsForCoord(Vec2f coord) {
	Point p1, p2;
	double rho = coord[0], theta = coord[1];
	double a = cos(theta), b = sin(theta);
	double x0 = a*rho, y0 = b*rho;
	p1.x = cvRound(x0 + 1000 * (-b));
	p1.y = cvRound(y0 + 1000 * (a));
	p2.x = cvRound(x0 - 1000 * (-b));
	p2.y = cvRound(y0 - 1000 * (a));
	return vector < Point > {p1, p2};
}

Point ROI::findIntersectionPoint(Point p1Line1, Point p2Line1, Point p1Line2, Point p2Line2) {
	vector<double> paramLeft, paramRight;
	paramLeft = computeLineParam(p1Line1, p2Line1);
	paramRight = computeLineParam(p1Line2, p2Line2);
	double al = paramLeft[0], bl = paramLeft[1], cl = paramLeft[2];
	double ar = paramRight[0], br = paramRight[1], cr = paramRight[2];

	Point p;
	p.y = (ar * cl - al * cr) / (al * br - ar * bl);
	p.x = (-cl - bl * p.y) / al;
	return p;
}

vector<double> ROI::computeLineParam(Point p1, Point p2) {
	double a, b, c;
	a = p1.y - p2.y;
	b = p2.x - p1.x;
	c = p1.x * p2.y - p2.x * p1.y;
	vector<double> parameters;
	parameters.push_back(a);
	parameters.push_back(b);
	parameters.push_back(c);
	return parameters;
}

void ROI::drawROI(Mat image) {
	Mat dst = image.clone();
	polylines(dst, roiPoints, true, Scalar(255, 255, 255), 2, CV_AA);
	imshow("ROI", dst);
}


void ROI::findLineOnROI(Mat image) {
	Mat binarizedImage = binarization(image);
	Mat workingImage = closing(binarizedImage, 3);
	Mat coloredImage = makeImageColor(image);
	//left line
	Point p1 = roiPoints[0];
	Point p2 = roiPoints[1];
	int minY = min(p1.y, p2.y);
	int maxY = max(p1.y, p2.y);
	for (int y = minY; y < maxY; y++) {
		double x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
		if (isInRange(workingImage, y, x)) {
			for (int xOffset = x - 7; xOffset <= x + 7; xOffset++) {
				if (isInRange(workingImage, y, xOffset)) {
					if (workingImage.at<uchar>(y, xOffset) == 255) {
						Vec3b color = coloredImage.at<Vec3b>(Point(xOffset, y));
						color[0] = 0;
						color[1] = 255;
						color[2] = 0;
						coloredImage.at<Vec3b>(Point(xOffset, y)) = color;
					}
				}
			}
		}
	}

	//right line
	p1 = roiPoints[2];
	p2 = roiPoints[3];
	minY = min(p1.y, p2.y);
	maxY = max(p1.y, p2.y);
	for (int y = minY; y < maxY; y++) {
		double x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
		if (isInRange(workingImage, y, x)) {
			for (int xOffset = x - 7; xOffset <= x + 7; xOffset++) {
				if (isInRange(workingImage, y, xOffset)) {
					if (workingImage.at<uchar>(y, xOffset) == 255) {
						Vec3b color = coloredImage.at<Vec3b>(Point(xOffset, y));
						color[0] = 0;
						color[1] = 255;
						color[2] = 0;
						coloredImage.at<Vec3b>(Point(xOffset, y)) = color;
					}
				}
			}
		}
	}

	imshow("Lanes", coloredImage);
}
