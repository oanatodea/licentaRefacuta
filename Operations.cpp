#include "stdafx.h"
#include "Operations.h"


bool isInRange(Mat src, int i, int j) {
	if (i < 0) {
		return false;
	}
	if (i >= src.rows) {
		return false;
	}
	if (j < 0) {
		return false;
	}
	if (j >= src.cols) {
		return false;
	}
	return true;
}

Mat dilation(Mat src, vector<int> convMatrixI, vector<int> convMatrixJ) {
	Mat dst = src.clone();
	int i, j, convIndexI, convIndexJ;
	for (i = 0; i < src.rows; i++) {
		for (j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) == 255) {
				for (convIndexI = 0; convIndexI < convMatrixI.size(); convIndexI++) {
					for (convIndexJ = 0; convIndexJ < convMatrixJ.size(); convIndexJ++) {
						int newI = i + convMatrixI[convIndexI];
						int newJ = j + convMatrixJ[convIndexJ];
						if (isInRange(dst, newI, newJ)) {
							dst.at<uchar>(newI, newJ) = 255;
						}
					}
				}
			}
		}
	}
	return dst;
}

Mat erosion(Mat src, vector<int> convMatrixI, vector<int> convMatrixJ) {
	Mat dst = src.clone();
	int i, j, convIndexI, convIndexJ;
	for (i = 0; i < src.rows; i++) {
		for (j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) == 255) {
				bool hasBlackNeighbours = false;
				for (convIndexI = 0; convIndexI < convMatrixI.size() && !hasBlackNeighbours; convIndexI++) {
					for (convIndexJ = 0; convIndexJ < convMatrixJ.size() && !hasBlackNeighbours; convIndexJ++) {
						int newI = i + convMatrixI[convIndexI];
						int newJ = j + convMatrixJ[convIndexJ];
						if (isInRange(src, newI, newJ)) {
							if (src.at<uchar>(newI, newJ) == 0) {
								hasBlackNeighbours = true;
							}
						}
					}
				}
				if (hasBlackNeighbours) {
					dst.at<uchar>(i, j) = 0;
				}
			}
		}
	}
	return dst;
}

Mat opening(Mat src, const int convolutionSize) {
	vector<int> convMatrixI;
	vector<int> convMatrixJ;
	for (int i = -convolutionSize / 2; i <= convolutionSize / 2; i++) {
		convMatrixI.push_back(i);
		convMatrixJ.push_back(i);
	}
	src = erosion(src, convMatrixI, convMatrixJ);
	src = dilation(src, convMatrixI, convMatrixJ);
	//imshow("open", src);
	return src;
}

Mat closing(Mat src, const int convolutionSize) {
	vector<int> convMatrixI;
	vector<int> convMatrixJ;
	for (int i = -convolutionSize / 2; i <= convolutionSize / 2; i++) {
		convMatrixI.push_back(i);
		convMatrixJ.push_back(i);
	}
	src = dilation(src, convMatrixI, convMatrixJ);
	src = erosion(src, convMatrixI, convMatrixJ);
	//imshow("close", src);
	return src;
}

Mat etichetare(Mat src, Mat opened) {
	Mat dst = src.clone();
	int eticheta = 255;
	vector<int> offsetI = { -1, 0, 1 };
	vector<int> offsetJ = { -1, 0, 1 };
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = 0;
		}
	}
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) == 255 && opened.at<uchar>(i, j) == 255 && dst.at<uchar>(i, j) == 0) {
				eticheta--;
				deque<Point> points;
				Point p;
				p.x = j;
				p.y = i;
				points.push_back(p);
				while (!points.empty()) {
					Point p = points.front();
					points.pop_front();
					dst.at<uchar>(p.y, p.x) = eticheta;
					for (int indexI = 0; indexI < offsetI.size(); indexI++) {
						for (int indexJ = 0; indexJ < offsetJ.size(); indexJ++) {
							// todo where is vector[] change with at()
							// todo point*
							int newI = p.y + offsetI.at(indexI);
							int newJ = p.x + offsetJ.at(indexJ);
							if (isInRange(src, newI, newJ)) {
								if (src.at<uchar>(newI, newJ) == 255 && dst.at<uchar>(newI, newJ) == 0) {
									Point p;
									p.x = newJ;
									p.y = newI;
									points.push_back(p);
									dst.at<uchar>(p.y, p.x) = eticheta;
								}
							}
						}
					}
				}
			}
		}
	}
	//imshow("etichetare", dst);
	return dst;
}

Mat binarization(Mat src) {
	Mat dst = src.clone();
	int threshold = 170;
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			if (src.at<uchar>(i, j) > threshold) {
				dst.at<uchar>(i, j) = 255;
			}
			else {
				dst.at<uchar>(i, j) = 0;
			}
		}
	}
	return dst;
}

Mat makeImageColor(Mat src) {
	Mat colorImage;
	cvtColor(src, colorImage, CV_GRAY2BGR);
	return colorImage;
}


Mat otherMarkingsElimination(Mat src, int threshold, vector<Point>& markingPoints, int minLineWidth) {
	Mat dst = src.clone();
	double height = src.rows, width = src.cols;
	vector<int> offsetI = { -1, 0, 1 };
	vector<int> offsetJ = { -1, 0, 1 };
	int firstLineMarking = transversalMarking(src, threshold, minLineWidth);
	int minY = height, maxY = 0, minX = width, maxX = 0;
	if (firstLineMarking != -1 && firstLineMarking < height / 2) {
		int i = firstLineMarking;
		while (i < height && i <= firstLineMarking + 3) {
			for (int j = 0; j < width; j++) {
				if (dst.at<uchar>(i, j) != 0) {
					minY = height, maxY = 0, minX = width, maxX = 0;
					deque<Point> points;
					Point p;
					p.x = j;
					p.y = i;
					points.push_back(p);
					while (!points.empty()) {
						Point p = points.front();
						points.pop_front();
						dst.at<uchar>(p.y, p.x) = 0;
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
						for (int indexI = 0; indexI < offsetI.size(); indexI++) {
							for (int indexJ = 0; indexJ < offsetJ.size(); indexJ++) {
								int newI = p.y + offsetI.at(indexI);
								int newJ = p.x + offsetJ.at(indexJ);
								if (isInRange(dst, newI, newJ)) {
									if (dst.at<uchar>(newI, newJ) != 0) {
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
				}
			}
			i++;
		}
		markingPoints.push_back(Point(minX, maxY));
		markingPoints.push_back(Point(minX, minY));
		markingPoints.push_back(Point(maxX, minY));
		markingPoints.push_back(Point(maxX, maxY));
	}
	return dst;
}

int transversalMarking(Mat src, int threshold, int minLineWidth) {
	vector<int> whitePixels;
	double height = src.rows, width = src.cols;
	for (int i = 0; i < height; i++) {
		int sum = 0;
		for (int j = 0; j < width; j++) {
			if (src.at<uchar>(i, j) != 0) {
				sum++;
			}
		}
		whitePixels.push_back(sum);
	}
	int noOfLinesFound = 0;
	int noOfWhitePixels = 0;
	int whitePixelsError = 10;
	for (int i = 0; i < whitePixels.size(); i++) {
		if (whitePixels.at(i) >= threshold) {
			noOfLinesFound++;
			if (noOfLinesFound == 0) {
				noOfWhitePixels = whitePixels.at(i);
			}
			if (noOfLinesFound >= minLineWidth) {
				return i - noOfLinesFound + 1;
			}
		}
		else {
			if (noOfLinesFound != 0) {
				noOfLinesFound = 0;
				noOfWhitePixels = 0;
			}
		}
	}
	return -1;
}


bool checkMarking(vector<Point> markingPoints, int maxWidth) {
	if (!markingPoints.empty() && abs(markingPoints[0].y - markingPoints[1].y) > maxWidth) {
		return false;
	}
	return true;
}

Mat ucharToIntMat(Mat src) {
	Mat dst = Mat(src.rows, src.cols, DataType<int>::type);
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dst.at<int>(i, j) = (int)src.at<uchar>(i, j);
		}
	}
	return dst;
}


Mat intToUcharMat(Mat src) {
	Mat dst = Mat(src.rows, src.cols, DataType<uchar>::type);
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = (uchar)src.at<int>(i, j);
		}
	}
	return dst;
}


Mat ucharToLongMat(Mat src) {
	Mat dst = Mat(src.rows, src.cols, DataType<long>::type);
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dst.at<long>(i, j) = (long)src.at<uchar>(i, j);
		}
	}
	return dst;
}


Mat longToUcharMat(Mat src) {
	Mat dst = Mat(src.rows, src.cols, DataType<uchar>::type);
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dst.at<uchar>(i, j) = (uchar)src.at<long>(i, j);
		}
	}
	return dst;
}