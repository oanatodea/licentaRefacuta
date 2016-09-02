#include "stdafx.h"
#include "Main.h"
#include "CannyAlg.h"

#define INF LONG_MAX

void readTemplates();
Mat applyDTToTemplate(String filename);
Mat applyCanny(Mat src);
Mat findDT(Mat src);
Mat initForDT(Mat longSrc, Mat src);
Mat scanFromUpLeft(Mat src);
Mat scanFromDownRight(Mat src);
void findArrows(Mat src);
void conexeComponents(Mat src);
bool hasNeighbour(Mat src, int i, int j, Point *p, int maxDiffToSearchNeighbour);