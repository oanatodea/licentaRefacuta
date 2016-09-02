#include "stdafx.h"
#include "Main.h"
#include "CannyAlg.h"

#define INF 255

void readTemplates();
Mat applyDTToTemplate(String filename);
Mat applyCanny(Mat src);
Mat findDT(Mat src);
Mat initForDT(Mat src);
Mat scanFromUpLeft(Mat src);
Mat scanFromDownRight(Mat src);
void findArrows(Mat src);
void conexeComponents(Mat src);
bool hasNeighbour(Mat src, int i, int j, Point *p, int maxDiffToSearchNeighbour);
Mat extractRectangle(Rect rectangle, Mat src);
void matchTemplates(Mat src);
float findScore(Mat src, Mat templateDT, Mat srcDT);
float findMatchingScoreForImage(Mat image, Mat templateDT);
float findMatchingScoreForTemplate(Mat imageDT, Mat templateDT);
void writeToFile(String filename, Mat src);