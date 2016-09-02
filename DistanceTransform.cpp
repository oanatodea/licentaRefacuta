#include "stdafx.h"
#include "DistanceTransform.h"

Mat straightDT, leftDT, rightDT, straightLeftDT, straightRightDT;
enum ArrowType{STRAIGHT, LEFT, RIGHT, STRAIGHT_LEFT, STRAIGHT_RIGHT};
double templateWidth, templateHeight;

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

	writeToFile("straight.txt", straightDT);
	writeToFile("left.txt", leftDT);
	writeToFile("right.txt", rightDT);
	writeToFile("straightLeft.txt", straightLeftDT);
	writeToFile("straightRight.txt", straightRightDT);
}

Mat applyDTToTemplate(String filename) {
	Mat image = imread(filename);
	Mat grayImage;
	if (image.channels() == 3) {
		cvtColor(image, grayImage, CV_BGR2GRAY);
	}
	templateHeight = grayImage.rows;
	templateWidth = grayImage.cols;
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

Mat findDT(Mat src) {// = ucharToLongMat(src);
	Mat longMat = initForDT(src);
	longMat = scanFromUpLeft(longMat);
	longMat = scanFromDownRight(longMat);
	return longMat;
}

Mat initForDT(Mat src) {
	Mat dst = src.clone();
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) == 255) {
				dst.at<uchar>(i, j) = 0;
			}
			else {
				dst.at<uchar>(i, j) = INF;
			}
		}
	}
	return dst;
}

Mat scanFromUpLeft(Mat src) {
	vector<int> offsetI = { -1, -1, -1, 0, 0 };
	vector<int> offsetJ = { -1, 0, 1, -1, 0 };
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			long min = INF;
			for (int offsetIndex = 0; offsetIndex < offsetI.size(); offsetIndex++) {
				int newI = i + offsetI[offsetIndex];
				int newJ = j + offsetJ[offsetIndex];
				if (isInRange(src, newI, newJ)) {
					int weight;
					if (newI == i && newJ == j) {
						weight = 0;
					}
					else {
						weight = 1;
					}
					if ((src.at<uchar>(newI, newJ) + weight) < min) {
						min = src.at<uchar>(newI, newJ) + weight;
					}
				}
			}
			if (min != INF) {
				src.at<uchar>(i, j) = min;
			}
		}
	}
	return src;
}


Mat scanFromDownRight(Mat src) {
	vector<int> offsetI = { 0, 0, 1, 1, 1 };
	vector<int> offsetJ = { 0, 1, -1, 0, 1 };
	for (int i = src.rows - 1; i >= 0 ; i--) {
		for (int j = src.cols - 1; j >= 0;  j--) {
			long min = INF;
			for (int offsetIndex = 0; offsetIndex < offsetI.size(); offsetIndex++) {
				int newI = i + offsetI[offsetIndex];
				int newJ = j + offsetJ[offsetIndex];
				if (isInRange(src, newI, newJ)) {
					int weight;
					if (newI == i && newJ == j) {
						weight = 0;
					}
					else {
						weight = 1;
					}
					if ((src.at<uchar>(newI, newJ) + weight) < min) {
						min = src.at<uchar>(newI, newJ) + weight;
					}
				}
			}
			if (min != INF) {
				src.at<uchar>(i, j) = min;
			}
		}
	}
	return src;
}

void findArrows(Mat src) {
	conexeComponents(src);
}

void conexeComponents(Mat src) {
	Mat dst = src.clone();
	vector<int> offsetI = { -1, 0, 1 };
	vector<int> offsetJ = { -1, 0, 1 };
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			if (dst.at<uchar>(i, j) == 255) {
				int minY = src.rows, maxY = -1, minX = src.cols, maxX = -1;
				deque<Point> points;
				Point p;
				p.x = j;
				p.y = i;
				points.push_back(p);
				while (!points.empty()) {
					Point p = points.front();
					points.pop_front();
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
					dst.at<uchar>(p.y, p.x) = 0;
					for (int indexI = 0; indexI < offsetI.size(); indexI++) {
						for (int indexJ = 0; indexJ < offsetJ.size(); indexJ++) {
							int newI = p.y + offsetI.at(indexI);
							int newJ = p.x + offsetJ.at(indexJ);
							if (isInRange(src, newI, newJ)) {
								if (dst.at<uchar>(newI, newJ) == 255) {
									Point p;
									p.x = newJ;
									p.y = newI;
									points.push_back(p);
									dst.at<uchar>(p.y, p.x) = 0;
								}
							}
						}
					}
				}

				if (minY != src.rows && maxY != -1 && minX != src.cols && maxX != -1) {
					Rect rec(Point(minX, minY), Point(maxX, maxY));
					Mat imageRectangle = extractRectangle(rec, src);
					matchTemplates(imageRectangle);
					imshow("Image rect", imageRectangle);
					waitKey();
				}
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


Mat extractRectangle(Rect rectangle, Mat src) {
	Mat imageRect = src(rectangle);
	Mat imageResized;
	double fy = templateHeight / imageRect.rows;
	double fx = min(fy, templateWidth / imageRect.cols);
	resize(imageRect, imageResized, Size(), fx, fy, INTER_LINEAR);
	double dif = templateWidth - imageResized.cols;
	Mat imageWithBorder(templateHeight, templateWidth, CV_8UC1);
	copyMakeBorder(imageResized, imageWithBorder, 0, 0, dif / 2, dif / 2, BORDER_CONSTANT, 0);
	vector<vector<Point> > contours;
	findContours(imageWithBorder, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point());
	Mat dst = Mat::zeros(imageWithBorder.size(), CV_8UC1);
	for (int i = 0; i< contours.size(); i++)
	{
		drawContours(dst, contours, i, Scalar(255, 255, 255), 1, 8, noArray(), 0, Point());
	}
	
	return dst;
}

void matchTemplates(Mat src) {
	float scores[5];
	Mat srcDT = findDT(src);
	writeToFile("src.txt", srcDT);
	scores[0] = findScore(src, straightDT, srcDT);
	scores[1] = findScore(src, leftDT, srcDT);
	scores[2] = findScore(src, rightDT, srcDT);
	scores[3] = findScore(src, straightLeftDT, srcDT);
	scores[4] = findScore(src, straightRightDT, srcDT);
	float minScore = INF;
	int minScoreIndex = -1;
	for (int i = 0; i < 5; i++) {
		if (minScore > scores[i]) {
			minScore = scores[i];
			minScoreIndex = i;
		}
	}

	printf("Min score %f + type = %d\n", minScore, minScoreIndex);

}

float findScore(Mat src, Mat templateDT, Mat srcDT) {
	float scoreForSrc = findMatchingScoreForImage(src, templateDT);
	float scoreForTemplate = findMatchingScoreForTemplate(srcDT, templateDT);
	return max(scoreForSrc, scoreForTemplate);
}

float findMatchingScoreForImage(Mat image, Mat templateDT) {
	float score = 0.0;
	long noOfPoints = 0;
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if (image.at<uchar>(i, j) == 255) {
				if (score >= LONG_MAX - templateDT.at<uchar>(i, j)) {
					score = LONG_MAX;
				}
				else {
					score += templateDT.at<uchar>(i, j);
				}
				noOfPoints++;
			}
		}
	}
	if (noOfPoints == 0) {
		return 0;
	}
	return score / noOfPoints;
}


float findMatchingScoreForTemplate(Mat imageDT, Mat templateDT) {
	float score = 0.0;
	long noOfPoints = 0;
	for (int i = 0; i < templateDT.rows; i++) {
		for (int j = 0; j < templateDT.cols; j++) {
			if (templateDT.at<uchar>(i, j) == 0) {
				if (score >= LONG_MAX - templateDT.at<uchar>(i, j)) {
					score = LONG_MAX;
				}
				else {
					score += imageDT.at<uchar>(i, j);
				}
				noOfPoints++;
			}
		}
	}
	if (noOfPoints == 0) {
		return 0;
	}
	return score/noOfPoints;
}

void writeToFile(String filename, Mat src) {
	cv::FileStorage file(filename, cv::FileStorage::WRITE);
	file << "Src" << src;
	file.release();
}