#include "stdafx.h"
#include "common.h"
#include "CannyAlg.h"
#include "ROI.h"
#include "IPMAlg.h"

Mat openImage(char* fileName) {
	Mat src, grayImage;
	src = imread(fileName);

	if (src.channels() == 3) {
		cvtColor(src, grayImage, CV_BGR2GRAY);
		return grayImage;
	}
	return src;
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

void main() {
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src = openImage(fname);
		Mat intImage = ucharToIntMat(src);
		CannyAlg* margins = new CannyAlg(intImage);
		Mat cannyResult = (*margins).finalImage;
		Mat ucharCannyResult = intToUcharMat(cannyResult);
		//imshow("Canny", ucharCannyResult);

		ROI* roi = new ROI(ucharCannyResult);
		(*roi).drawROI(src);
		(*roi).findLineOnROI(src);

		IPMAlg* ipm = new IPMAlg(src, (*roi).roiPoints);
		Mat inverseImage = (*ipm).remapImage(src);
		//imshow("IPM", inverseImage);

		//operations on image
		Mat binarImage = binarization(inverseImage);
		Mat closedImage = closing(binarImage, 3);

		const int thresholdForZebra = src.cols / 3;
		const int thresholdForLine = src.cols / 2;
		vector<Point> zebraPoints, stopLinePoints;
		Mat imageWithoutStopLine = otherMarkingsElimination(closedImage, thresholdForLine, stopLinePoints, 5);
		Mat imageWithoutZebra = otherMarkingsElimination(imageWithoutStopLine, thresholdForZebra, zebraPoints, 7);

		Mat openedImage = opening(imageWithoutZebra, 9);
		Mat finalClosedImage = closing(openedImage, 13);
		imshow("Closed", finalClosedImage);


		//draw rectangle to show markings 
		Mat colorImage = makeImageColor(inverseImage);
		if (checkMarking(stopLinePoints, 30)) {
			polylines(colorImage, stopLinePoints, true, Scalar(0, 255, 255), 1, 8, 0);
		}
		if (checkMarking(zebraPoints, src.rows / 2)) {
			polylines(colorImage, zebraPoints, true, Scalar(255, 255, 0), 1, 8, 0);
		}
		//imshow("Final image", colorImage);
		waitKey();
	}
}