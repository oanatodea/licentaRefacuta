#include "stdafx.h"
#include "Main.h"

class CannyAlg {
public: 
	CannyAlg(Mat src);
	Mat finalImage;
private:
	int height, width;
	int ignoreMargin;
	
	Mat convolution(Mat src, std::vector< std::vector<double>> filter, std::vector<double> offsetI, std::vector<double> offsetJ);
	Mat gaussFiltration(cv::Mat src);
	void computeGradientModuleAndDirection(Mat src, Mat* mod, Mat* dir);
	int quantifyDirection(double value);
	Mat suprimateNonMaxim(Mat module, Mat dir);
	int adaptiveThresholding(Mat module);
	Mat hysteresis(Mat mod, int adaptiveThreshold);
	Mat edgeLinking(Mat src);
};