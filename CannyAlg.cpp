#include "stdafx.h"
#include "CannyAlg.h"

struct point{
	int i;
	int j;
};

CannyAlg::CannyAlg(Mat src) {
	width = src.cols;
	height = src.rows;
	Mat noiseFiltration = gaussFiltration(src);
	Mat mod, dir;
	computeGradientModuleAndDirection(noiseFiltration, &mod, &dir);
	Mat nonMaxSuprimated = suprimateNonMaxim(mod, dir);
	finalImage = edgeLinking(nonMaxSuprimated);
}

Mat CannyAlg::gaussFiltration(Mat src) {
	std::vector< std::vector<double>> filter = { { 1.0 / 16, 1.0 / 8, 1.0 / 16 }, { 1.0 / 8, 1.0 / 4, 1.0 / 8 }, { 1.0 / 16, 1.0 / 8, 1.0 / 16 } };
	/*vector< std::vector<double>> filter = {
	{ 1.0 / 273, 4.0 / 273, 7.0 / 273, 4.0 / 273, 1.0 / 273},
	{ 4.0 / 273, 16.0 / 273, 26.0 / 273, 16.0 / 273, 4.0 / 273 },
	{ 7.0 / 273, 26.0 / 273, 41.0 / 273, 26.0 / 273, 7.0 / 273 },
	{ 4.0 / 273, 16.0 / 273, 26.0 / 273, 16.0 / 273, 4.0 / 273 },
	{ 1.0 / 273, 4.0 / 273, 7.0 / 273, 4.0 / 273, 1.0 / 273 } };*/
	std::vector<double> offsetI = { -1, 0, 1 };
	std::vector<double> offsetJ = { -1, 0, 1 };
	return convolution(src, filter, offsetI, offsetJ);
}

Mat CannyAlg::convolution(Mat src, std::vector< std::vector<double>> filter, std::vector<double> offsetI, std::vector<double> offsetJ) {
	Mat dst = Mat(height, width, DataType<int>::type);
	ignoreMargin = max_(ignoreMargin, filter.size() / 2);
	for (int i = ignoreMargin; i < height - ignoreMargin; i++) {
		for (int j = ignoreMargin; j < width - ignoreMargin; j++) {
			double newValue = 0;
			for (int indexIFilter = 0; indexIFilter < offsetI.size(); indexIFilter++) {
				for (int indexJFilter = 0; indexJFilter < offsetJ.size(); indexJFilter++) {
					int newI = i + offsetI[indexIFilter];
					int newJ = j + offsetJ[indexJFilter];
					if (isInRange(src, newI, newJ)) {
						double value = src.at<int>(newI, newJ);
						newValue = newValue + filter[indexIFilter][indexJFilter] * value;
					}
				}
			}
			dst.at<int>(i, j) = newValue;
		}
	}
	return dst;
}

void CannyAlg::computeGradientModuleAndDirection(Mat src, Mat* mod, Mat* dir) {
	*mod = src.clone();
	*dir = src.clone();
	std::vector< std::vector<double>> filterX = { { -1, 0, 1 }, { -2, 0, 2 }, { -1, 0, 1 } };
	std::vector< std::vector<double>> filterY = { { 1, 2, 1 }, { 0, 0, 0 }, { -1, -2, -1 } };
	std::vector<double> offsetI = { -1, 0, 1 };
	std::vector<double> offsetJ = { -1, 0, 1 };

	Mat convX = convolution(src, filterX, offsetI, offsetJ);
	Mat convY = convolution(src, filterY, offsetI, offsetJ);

	for (int i = ignoreMargin; i < height - ignoreMargin; i++) {
		for (int j = ignoreMargin; j < width - ignoreMargin; j++) {
			double moduleValue = sqrt(pow(convX.at<int>(i, j), 2.0) + pow(convY.at<int>(i, j), 2.0));
			(*mod).at<int>(i, j) = moduleValue;
			double dirValue = std::atan2(convY.at<int>(i, j), convX.at<int>(i, j)) * 180.0 / M_PI;
			(*dir).at<int>(i, j) = quantifyDirection(dirValue);
		}
	}
}

int CannyAlg::quantifyDirection(double value) {
	double halfPeriod = 45.0 / 2;
	double raport = value / halfPeriod;
	if ((raport >= -1 && raport < 1) || (raport >= 7 && raport <= 8) || (raport >= -8 && raport < -7)) {
		return 2;
	}
	if ((raport >= 1 && raport < 3) || (raport >= -7 && raport < -5)) {
		return 1;
	}
	if ((raport >= 3 && raport < 5) || (raport >= -5 && raport < -3)) {
		return 0;
	}
	if ((raport >= 5 && raport < 7) || (raport >= -3 && raport < -1)) {
		return 3;
	}
	return -1;
}

Mat CannyAlg::suprimateNonMaxim(Mat module, Mat dir) {
	// destination has to be a module clone
	Mat dst = module.clone();
	std::vector<double> offsetI = { 1, -1, 0, 1 };
	std::vector<double> offsetJ = { 0, 1, 1, 1 };
	for (int i = ignoreMargin; i < height - ignoreMargin; i++) {
		for (int j = ignoreMargin; j < width - ignoreMargin; j++) {
			int quantifiedDirection = dir.at<int>(i, j);
			if (quantifiedDirection == -1) {
				printf("Error ! dir = %d\n", dir.at<int>(i, j));
				break;
			}
			if (module.at<int>(i, j) != 0) {
				int newI1 = i + offsetI[quantifiedDirection];
				int newJ1 = j + offsetJ[quantifiedDirection];
				int newI2 = i - offsetI[quantifiedDirection];
				int newJ2 = j - offsetJ[quantifiedDirection];
				if (isInRange(module, newI1, newJ1)) {
					if (module.at<int>(i, j) <= module.at<int>(newI1, newJ1)) {
						dst.at<int>(i, j) = 0;
					}
				}
				if (isInRange(module, newI2, newJ2)) {
					if (module.at<int>(i, j) <= module.at<int>(newI2, newJ2)) {
						dst.at<int>(i, j) = 0;
					}
				}
			}
		}
	}
	return dst;
}

Mat CannyAlg::edgeLinking(Mat src) {
	int adaptiveThreshold = adaptiveThresholding(src);
	return hysteresis(src, adaptiveThreshold);
}


int CannyAlg::adaptiveThresholding(Mat module) {
	const double p = 0.01;
	int *hist = new int[256]();
	for (int i = ignoreMargin; i < height - ignoreMargin; i++) {
		for (int j = ignoreMargin; j < width - ignoreMargin; j++) {
			int normalizedValue = module.at<int>(i, j) / (4 * sqrt(2));
			hist[normalizedValue]++;
		}
	}
	int nrNonMuchii = (1 - p) * ((width - 2 * ignoreMargin) * (height - 2 * ignoreMargin) - hist[0]);
	int adaptiveThreshold = 0;
	int sum = 0;
	for (int i = 1; i <= 255; i++) {
		sum += hist[i];
		if (sum >= nrNonMuchii) {
			adaptiveThreshold = i;
			break;
		}
	}
	return adaptiveThreshold;
}

Mat CannyAlg::hysteresis(Mat mod, int adaptiveThreshold) {
	Mat dst = mod.clone();
	const double k = 0.7;
	const int muchieTare = 255;
	const int muchieSlaba = 128;
	const int nonMuchie = 0;
	double lowThreshold = k * adaptiveThreshold;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (i == 0 || j == 0 || i == height - 1 || j == width - 1) {
				dst.at<int>(i, j) = nonMuchie;
			}
			else if (mod.at<int>(i, j) >= adaptiveThreshold) {
				dst.at<int>(i, j) = muchieTare;
			}
			else if (mod.at<int>(i, j) >= lowThreshold) {
				dst.at<int>(i, j) = muchieSlaba;
			}
			else {
				dst.at<int>(i, j) = nonMuchie;
			}
		}
	}
	std::vector<double> offsetI = { -1, 0, 1 };
	std::vector<double> offsetJ = { -1, 0, 1 };
	std::deque<point*> points;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (dst.at<int>(i, j) == muchieTare) {
				point *p = new point();
				p->i = i;
				p->j = j;
				points.push_back(p);
				while (!points.empty()) {
					point *newP = points.front();
					points.pop_front();
					for (int i = 0; i < offsetI.size(); i++) {
						for (int j = 0; j < offsetJ.size(); j++) {
							int newI = newP->i + offsetI[i];
							int newJ = newP->j + offsetJ[j];
							if (isInRange(dst, newI, newJ)) {
								if (dst.at<int>(newI, newJ) == muchieSlaba) {
									dst.at<int>(newI, newJ) = muchieTare;
									points.push_back(newP);
								}
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (dst.at<int>(i, j) == muchieSlaba) {
				dst.at<int>(i, j) = nonMuchie;
			}
		}
	}
	return dst;
}