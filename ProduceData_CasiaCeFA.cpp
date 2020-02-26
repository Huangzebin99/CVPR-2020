#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include <Eigen/Dense>
#include <direct.h>
#include <sys/timeb.h>
//#include <vector>
//#include <iomanip>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "NL_err.h"
#include "NL_FaceDetection.h"
#include "NL_Tools.h"

using namespace std;
using namespace cv;
using namespace Eigen;

void gAddBrightPoint(const Mat& djSrcImg,Mat& djDstImg,const Mat& djBackImg)
{
	djDstImg = djSrcImg.clone();
	int dwNumCircle = rand() % 20 + 280;

	int dwAllRows = djSrcImg.rows;
	int dwAllCols = djSrcImg.cols;

	int Ratio = rand() % 30 + 1;

	for (int i = 0;i < dwNumCircle;i++)
	{
		int dwTempRows = rand() % dwAllRows;
		int dwTempCols = rand() % dwAllCols;

		if (djBackImg.at<uchar>(dwTempRows, dwTempCols))
		{
			circle(djDstImg, Point(dwTempCols, dwTempRows), dwAllCols / (rand() % 20 + 40), Scalar(220+rand()%36), -1);
		}
		else
		{
			i--;
		}
	}
}

int main()
{
	// Augmentation IR
	ifstream ftp("train/list_rgb.txt");
	string lines;

	while (getline(ftp, lines))
	{
		if (lines.find("profile") != string::npos)
		{
			Mat djSrcImg = imread(lines);
			if (djSrcImg.empty())
			{
				printf("Can not load all image!\n");
				continue;
			}
			
			cvtColor(djSrcImg, djSrcImg, CV_BGR2GRAY);

			// get background area
			Mat djMaskBack;
			threshold(djSrcImg, djMaskBack, 0, 255, CV_THRESH_BINARY);

			// store gray rgb
			if (0 == rand() % 2)	// 不进行模糊操作
			{
				// add bright spot in rand area
				if (0 == rand() % 2)
				{
					//Mat djDstImg;
					gAddBrightPoint(djSrcImg, djSrcImg, djMaskBack);
				}
			}
			else
			{
				// add bright spot in rand area
				if (0 == rand() % 2)
				{
					//Mat djDstImg;
					gAddBrightPoint(djSrcImg, djSrcImg, djMaskBack);

					// blur
					int dwKerSize = rand() % 5 + 5;
					dwKerSize = dwKerSize % 2 == 0 ? dwKerSize - 1 : dwKerSize;
					int dwStd = rand() % 5 + 5;
					GaussianBlur(djSrcImg, djSrcImg, Size(dwKerSize, dwKerSize), dwStd, dwStd);
				}
				else
				{
					// blur
					int dwKerSize = rand() % 5 + 5;
					dwKerSize = dwKerSize % 2 == 0 ? dwKerSize - 1 : dwKerSize;
					int dwStd = rand() % 5 + 5;
					GaussianBlur(djSrcImg, djSrcImg, Size(dwKerSize, dwKerSize), dwStd, dwStd);
				}
			}

			Mat djBacImg(djSrcImg.size() * 3, CV_8UC1, Scalar(0, 0, 0));
			djSrcImg.copyTo(djBacImg(Rect(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows)));

			// Move Dis
			Rect FaceR(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows);
			int dwExpNum = 2;
			int dwMoveDis = max(FaceR.width, FaceR.height) / (rand() % 10 + 3);
			int dwCore = rand() % 180 - 90;
			FaceR.x = FaceR.x + cos(dwCore)*dwMoveDis;
			FaceR.y = FaceR.y + sin(dwCore)*dwMoveDis;
			Mat djSrcImg1 = djBacImg(FaceR).clone();

			string StoreName = lines.substr(0, lines.length() - 4) + "_attack_1.jpg";
			cvtColor(djSrcImg1, djSrcImg1, CV_GRAY2BGR);
			imwrite(StoreName, djSrcImg1);

			// Rotate 
			Point2f pt(djSrcImg.cols / 2., djSrcImg.rows / 2.);
			double angle = -1;
			if (rand() % 2 == 0)
				angle = -(rand() % 20);
			else
				angle = (rand() % 20);
			Mat r = getRotationMatrix2D(pt, angle, 1.0);

			Mat djSrcImg2;
			warpAffine(djSrcImg, djSrcImg2, r, djSrcImg.size());

			string StoreName1 = lines.substr(0, lines.length() - 4) + "_attack_2.jpg";
			cvtColor(djSrcImg2, djSrcImg2, CV_GRAY2BGR);
			imwrite(StoreName1, djSrcImg2);
		}
		else
		{
			Mat djSrcImg = imread(lines);
			if (djSrcImg.empty())
			{
				printf("Can not load all image!\n");
				continue;
			}

			Mat djBacImg(djSrcImg.size() * 3, CV_8UC3, Scalar(0, 0, 0));
			djSrcImg.copyTo(djBacImg(Rect(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows)));

			// Move Dis
			Rect FaceR(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows);
			int dwExpNum = 2;
			int dwMoveDis = max(FaceR.width, FaceR.height)/(rand()%10 + 10);
			int dwCore = rand() % 180 - 90;
			FaceR.x = FaceR.x + cos(dwCore)*dwMoveDis;
			FaceR.y = FaceR.y + sin(dwCore)*dwMoveDis;
			Mat djSrcImg1 = djBacImg(FaceR).clone();

			string StoreName = lines.substr(0, lines.length() - 4) + "_supply_real_1.jpg";
			imwrite(StoreName, djSrcImg1);

			// Rotate 
			Point2f pt(djSrcImg.cols / 2., djSrcImg.rows / 2.);
			double angle = -1;
			if (rand() % 2 == 0)
				angle = -(rand() % 20);
			else
				angle = (rand() % 20);
			Mat r = getRotationMatrix2D(pt, angle, 1.0);

			Mat djSrcImg2;
			warpAffine(djSrcImg, djSrcImg2, r, djSrcImg.size());

			string StoreName1 = lines.substr(0, lines.length() - 4) + "_supply_real_2.jpg";
			imwrite(StoreName1, djSrcImg2);
		}		
	}
	ftp.close();
}