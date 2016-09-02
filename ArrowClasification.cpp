#include "stdafx.h"
#include "ArrowClassification.h"

ArrowClassification::ArrowClassification(Mat grayImage, Mat colorImage) {
	src = grayImage;
	width = src.cols;
	height = src.rows;
	colorImageToDraw = colorImage;
	harris();
}

void ArrowClassification::harris() {
	vector<Point> corners;
	vector<pair<Point, int>> cornersWithLabels;
	Mat emptyImage(src.rows, src.cols, CV_8UC1, Scalar(0));
	goodFeaturesToTrack(src, corners, 20, 0.1, 10, noArray(), 5, true, 0.04);
	while (!corners.empty()) {
		Point p = corners.back();
		corners.pop_back();
		int label = src.at<uchar>(p.y, p.x);
		if (label == 0) {
			bool labelFound = false;
			// find label through neighbours
			for (int i = -2; i <= 2 && !labelFound; i++) {
				for (int j = -2; j <= 2 && !labelFound; j++) {
					int newI = p.y + i;
					int newJ = p.x + j;
					if (isInRange(src, newI, newJ)) {
						if (src.at<uchar>(newI, newJ) != 0) {
							label = src.at<uchar>(newI, newJ);
							labelFound = true;
						}
					}
				}
			}
		}
		cornersWithLabels.push_back(make_pair(p, label));

		if (label != 0) {
			circle(colorImageToDraw, p, 5, Scalar(0, 0, 255), 2, 8, 0);
			//emptyImage.at<uchar>(p) = 255;
		}
	}
	//vector<Vec4i> houghLines;
	//HoughLinesP(emptyImage, houghLines, 1, M_PI * 2 / 180, 2, 10, 100);
	//for (int i = 0; i < houghLines.size(); i++) {
		//line(emptyImage, Point(houghLines[i][0], houghLines[i][1]), Point(houghLines[i][2], houghLines[i][3]), Scalar(255), 2, 8, 0);
	//}
	//imshow("Harris", emptyImage);

	//cornersClasification(cornersWithLabels);
	imshow("Final", colorImageToDraw);
}

void ArrowClassification::cornersClasification(vector<pair<Point, int>> cornersWithLabels) {
	for (int i = 0; i < cornersWithLabels.size(); i++) {
		if (cornersWithLabels.at(i).second != -1) {
			vector<Point> cornersForLabel;
			cornersForLabel.push_back(cornersWithLabels.at(i).first);
			int currentLabel = cornersWithLabels.at(i).second;
			for (int j = i + 1; j < cornersWithLabels.size(); j++) {
				if (cornersWithLabels.at(j).second == currentLabel){
					cornersForLabel.push_back(cornersWithLabels.at(j).first);
					cornersWithLabels.at(j).second = -1;
				}
			}
			cornersWithLabels.at(i).second = -1;
			if (cornersForLabel.size() >= 5) {
				classification(cornersForLabel);
			}
		}
	}
}


void ArrowClassification::classification(vector<Point> corners) {
	const double distThreshold = 7;
	const double distBetweenPointsThreshold = 20;
	const int inlinersThreshold = 4;
	int currentInliners;
	bool inlinersFound = false;
	for (int i = 0; i < corners.size() && !inlinersFound; i++) {
		double a, b, c;
		Point p1 = corners.at(i);
		Point p2;
		p2 = corners.at((i + 1) % corners.size());
		currentInliners = 2;
		double distBetweenPoints = sqrt(pow(p1.x - p2.x, 2.0) + pow(p1.y - p2.y, 2.0));
		if (distBetweenPoints <= distBetweenPointsThreshold) {
			vector<int> indexInliners;
			indexInliners.push_back(i);
			indexInliners.push_back((i + 1) % corners.size());
			a = p1.y - p2.y;
			b = p2.x - p1.x;
			c = p1.x * p2.y - p2.x * p1.y;
			double num = sqrt(a * a + b * b);
			for (int j = 0; j < corners.size() && !inlinersFound; j++) {
				if (j != i && j != ((i + 1) % corners.size())) {
					Point p = corners.at(j);
					double dist = abs(a * p.x + b * p.y + c) / num;
					if (dist <= distThreshold) {
						currentInliners++;
						indexInliners.push_back(j);
						if (currentInliners == inlinersThreshold) {
							inlinersFound = true;
							line(colorImageToDraw, corners.at(indexInliners.at(0)), corners.at(indexInliners.at(1)), Scalar(255, 0, 0), 1, 8, 0);
							ArrowType arrowType = findOrientation(corners, indexInliners);
							drawRectangle(corners, arrowType);
						}
					}
				}
			}
		}
	}
}


ArrowClassification::ArrowType ArrowClassification::findOrientation(vector<Point> corners, vector<int> indexCollinearPoints) {
	Point p1 = corners.at(indexCollinearPoints.at(0));
	Point p2 = corners.at(indexCollinearPoints.at(1));
	const double errorAngle = 30;
	double angle = abs(atan2(abs(p2.y - p1.y), abs(p2.x - p1.x)) * 180 / M_PI);
	if ((180 - angle) <= errorAngle || angle <= errorAngle) {
		//horizontal line
		int pointsInUpperPlane = 0, pointsInLowerPlane = 0;
		for (int i = 0; i < corners.size(); i++) {
			if (!contains(indexCollinearPoints, i)) {
				if (isUp(p1, p2, corners.at(i))) {
					pointsInUpperPlane++;
				}
				else {
					pointsInLowerPlane++;
				}
			}
		}
		if (pointsInUpperPlane == 1) {
			return UP;
		}
		if (pointsInLowerPlane == 1) {
			return DOWN;
		}
	}
	if (abs(angle - 90) <= errorAngle) {
		//vertical line
		int pointsInLeftPlane = 0, pointsInRightPlane = 0;
		for (int i = 0; i < corners.size(); i++) {
			if (!contains(indexCollinearPoints, i)) {
				if (isLeft(p1, p2, corners.at(i))) {
					pointsInLeftPlane++;
				}
				else {
					pointsInRightPlane++;
				}
			}
		}
		if (pointsInLeftPlane == 1) {
			return LEFT;
		}
		if (pointsInRightPlane == 1) {
			return RIGHT;
		}
	}
}

void ArrowClassification::drawRectangle(vector<Point> corners, ArrowType arrowType) {
	int minX = width, minY = height, maxX = 0, maxY = 0;
	for (int i = 0; i < corners.size(); i++) {
		Point p = corners.at(i);
		if (p.x < minX) {
			minX = p.x;
		}
		if (p.x > maxX) {
			maxX = p.x;
		}
		if (p.y < minY) {
			minY = p.y;
		}
		if (p.y > maxY) {
			maxY = p.y;
		}
	}
	vector<Point> points;
	points.push_back(Point(minX - 1, maxY + 1));
	points.push_back(Point(minX + 1, minY - 1));
	points.push_back(Point(maxX + 1, minY - 1));
	points.push_back(Point(maxX + 1, maxY + 1));
	Scalar color = { 255, 255, 255 };
	if (arrowType == LEFT) {
		color = { 255, 0, 0 };
	}
	if (arrowType == RIGHT) {
		color = { 0, 0, 255 };
	}
	if (arrowType == UP) {
		color = { 0, 255, 0 };
	}
	polylines(colorImageToDraw, points, true, color, 2, CV_AA);
}

bool ArrowClassification::isLeft(Point a, Point b, Point c) {
	if (c.x < a.x || c.x < b.x) {
		return true;
	}
	return false;
}

bool ArrowClassification::isUp(Point a, Point b, Point c) {
	if (c.y < a.y || c.y < b.y) {
		return true;
	}
	return false;
}

bool ArrowClassification::contains(vector<int> vector, int x) {
	for (int i = 0; i < vector.size(); i++) {
		if (vector.at(i) == x) {
			return true;
		}
	}
	return false;
}
