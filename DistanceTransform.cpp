#include "stdafx.h"
#include "DistanceTransform.h"

Mat straightDT, leftDT, rightDT, straightLeftDT, straightRightDT;

void readTemplates() {
	String straightFileName = "straight.bmp";
	String leftFileName = "left.bmp";
	String rightFileName = "right.bmp";
	String straightLeftFileName = "straightLeft.bmp";
	String straightRightFileName = "straightRight.bmp";

	straightDT = applyDTToTemplate(straightFileName);
	leftDT = applyDTToTemplate(leftFileName);
	rightDT = applyDTToTemplate(rightFileName);
	straightLeftDT = applyDTToTemplate(straightLeftFileName);
	straightRightDT = applyDTToTemplate(straightRightFileName);
}

Mat applyDTToTemplate(String filename) {
	Mat image = cv::imread(filename);
	Mat grayImage;
	if (image.channels() == 3) {
		cvtColor(image, grayImage, CV_BGR2GRAY);
	}
	Mat cannyResult = applyCanny(grayImage);
	imshow(filename, cannyResult);
	Mat dt = findDT(cannyResult);
	return dt;
}

Mat applyCanny(Mat src) {
	Mat intSrc = ucharToIntMat(src);
	CannyAlg* canny = new CannyAlg(intSrc);
	Mat cannyResult = (*canny).finalImage;
	Mat ucharCannyResult = intToUcharMat(cannyResult);
	return ucharCannyResult;
}

Mat findDT(Mat src) {
	Mat longMat = ucharToLongMat(src);
	longMat = initForDT(longMat, src);
	longMat = scanFromUpLeft(longMat);
	longMat = scanFromDownRight(longMat);
	return longMat;
}

Mat initForDT(Mat longSrc, Mat src) {
	for (int i = 0; i < longSrc.rows; i++) {
		for (int j = 0; j < longSrc.cols; j++) {
			if (src.at<uchar>(i, j) == 255) {
				longSrc.at<long>(i, j) = 0;
			}
			else {
				longSrc.at<long>(i, j) = INF;
			}
		}
	}
	return longSrc;
}

Mat scanFromUpLeft(Mat src) {
	vector<int> offsetI = { -1, -1, -1, 0, 0 };
	vector<int> offsetJ = { -1, 0, 1, -1, 0 };
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			float min = INF;
			for (int offsetIndexI = 0; offsetIndexI < offsetI.size(); offsetIndexI++) {
				for (int offsetIndexJ = 0; offsetIndexJ < offsetJ.size(); offsetIndexJ++) {
					int newI = i + offsetI[offsetIndexI];
					int newJ = j + offsetJ[offsetIndexJ];
					if (isInRange(src, newI, newJ)) {
						if (src.at<long>(i, j) < min) {
							min = src.at<long>(i, j);
						}
					}
				}
			}
			if (min != INF) {
				src.at<long>(i, j) = min + 1;
			}
		}
	}
	return src;
}


Mat scanFromDownRight(Mat src) {
	vector<int> offsetI = { -1, -1, 1, 1, 1 };
	vector<int> offsetJ = { 0, 1, -1, 0, 1 };
	for (int i = src.rows - 1; i >= 0 ; i--) {
		for (int j = src.cols - 1; j >= 0;  j--) {
			float min = INF;
			for (int offsetIndexI = 0; offsetIndexI < offsetI.size(); offsetIndexI++) {
				for (int offsetIndexJ = 0; offsetIndexJ < offsetJ.size(); offsetIndexJ++) {
					int newI = i + offsetI[offsetIndexI];
					int newJ = j + offsetJ[offsetIndexJ];
					if (isInRange(src, newI, newJ)) {
						if (src.at<long>(i, j) < min) {
							min = src.at<long>(i, j);
						}
					}
				}
			}
			if (min != INF) {
				src.at<long>(i, j) = min + 1;
			}
		}
	}
	return src;
}

void findArrows(Mat src) {
	Mat canny = applyCanny(src);
	conexeComponents(canny);
}

void conexeComponents(Mat src) {
	Mat dst = src.clone();
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			if (dst.at<uchar>(i, j) == 255) {
				dst.at<uchar>(i, j) = 0;
				vector<Point> points;
				Point p;
				while (hasNeighbour(dst, i, j, &p, 5)) {
					points.push_back(p);
					dst.at<uchar>(p.y, p.x) = 0;
					i = p.y;
					j = p.x;
				}
				Rect rec = boundingRect(points);
				rectangle(src, rec, Scalar(255, 255, 255), 1, 8, 0);
			}
		}
	}
	imshow("Comp conexe", src);
}

bool hasNeighbour(Mat src, int currentI, int currentJ, Point *p, int maxDiffToSearchNeighbour) {
	for (int i = -maxDiffToSearchNeighbour; i <= maxDiffToSearchNeighbour; i++) {
		for (int j = -maxDiffToSearchNeighbour; j <= maxDiffToSearchNeighbour; j++) {
			int newI = currentI + i;
			int newJ = currentJ + j;
			if (isInRange(src, newI, newJ)) {
				if (src.at<uchar>(newI, newJ) == 255) {
					(*p).x = newJ;
					(*p).y = newI;
					return true;
				}
			}
		}
	}
	return false;
}


void remap(Rect rectangle, Mat src) {
	Mat dst;
	Mat aux()
	resize(rectangle, dst, straightDT.size(), 0, 0, INTER_LINEAR);
}