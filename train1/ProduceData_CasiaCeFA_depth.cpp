#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include <direct.h>
#include <sys/timeb.h>
//#include <vector>
//#include <iomanip>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

void gAddBrightPoint(const Mat& djSrcImg, Mat& djDstImg, const Mat& djBackImg)
{
	djDstImg = djSrcImg.clone();

	int dwNumCircle = rand() % 20 + 280;
	int dwAllRows = djSrcImg.rows;
	int dwAllCols = djSrcImg.cols;

	for (int i = 0;i < dwNumCircle;i++)
	{
		int dwTempRows = rand() % dwAllRows;
		int dwTempCols = rand() % dwAllCols;

		if (djBackImg.at<uchar>(dwTempRows, dwTempCols))
		{
			circle(djDstImg, Point(dwTempCols, dwTempRows), dwAllCols / (rand() % 20 + 40), Scalar(220 + rand() % 36), -1);
		}
		else
		{
			i--;
		}
	}
}

int main()
{
	// Augmentation Depth
	ifstream ftp("train/list_depth.txt");
	string lines;

	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
	compression_params.push_back(IMWRITE_PNG_STRATEGY);
	compression_params.push_back(IMWRITE_PNG_STRATEGY_DEFAULT);

	while (getline(ftp, lines))
	{
		if (lines.find("real") != string::npos)
		{
			Mat djSrcImg = imread(lines);
			Mat djSrcImg1 = djSrcImg.clone();
			int dwCount = 0;
			int dwCountValue = 0;
			for (int i = 0;i < djSrcImg.rows;i++)
			{
				for (int j = 0;j < djSrcImg.cols;j++)
				{
					if (0 != djSrcImg.at<Vec3b>(i, j)[0])
					{
						dwCountValue += djSrcImg.at<Vec3b>(i, j)[0];
						dwCount++;
					}
				}
			}

			int dwBig = rand() % 2 + 8;
			for (int i = 0;i < djSrcImg1.rows;i++)
			{
				for (int j = 0;j < djSrcImg1.cols;j++)
				{
					if (djSrcImg1.at<Vec3b>(i, j)[0]>20)
					{
						djSrcImg1.at<Vec3b>(i, j)[0] = djSrcImg1.at<Vec3b>(i, j)[0] * dwBig / 10;
						djSrcImg1.at<Vec3b>(i, j)[1] = djSrcImg1.at<Vec3b>(i, j)[1] * dwBig / 10;
						djSrcImg1.at<Vec3b>(i, j)[2] = djSrcImg1.at<Vec3b>(i, j)[2] * dwBig / 10;
					}
				}
			}

			string StoreName1 = lines.substr(0, lines.length() - 4) + "_aug_1.jpg";
			imwrite(StoreName1, djSrcImg1, compression_params);
			
			// Move Dis
			Mat djBacImg(djSrcImg.size() * 3, CV_8UC3, Scalar(0, 0, 0));
			djSrcImg.copyTo(djBacImg(Rect(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows)));

			Rect FaceR(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows);
			int dwMoveDis = max(FaceR.width, FaceR.height) / (rand() % 10 + 5);
			int dwCore = rand() % 180 - 90;
			FaceR.x = FaceR.x + cos(dwCore)*dwMoveDis;
			FaceR.y = FaceR.y + sin(dwCore)*dwMoveDis;
			Mat djSrcImg2 = djBacImg(FaceR).clone();

			string StoreName2 = lines.substr(0, lines.length() - 4) + "_aug_2.jpg";
			imwrite(StoreName2, djSrcImg2, compression_params);

			// Rotate 
			Point2f pt(djSrcImg.cols / 2., djSrcImg.rows / 2.);
			double angle = -1;
			if (rand() % 2 == 0)
				angle = -(rand() % 20);
			else
				angle = (rand() % 20);
			Mat r = getRotationMatrix2D(pt, angle, 1.0);

			Mat djSrcImg3;
			warpAffine(djSrcImg, djSrcImg3, r, djSrcImg.size());

			string StoreName3 = lines.substr(0, lines.length() - 4) + "_aug_3.jpg";
			imwrite(StoreName3, djSrcImg3, compression_params);
		}
		else
		{
			Mat djSrcImg = imread(lines);

			////////////////////////////
			// Move Dis
			Mat djBacImg(djSrcImg.size() * 3, CV_8UC3, Scalar(0, 0, 0));
			djSrcImg.copyTo(djBacImg(Rect(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows)));

			Rect FaceR(djSrcImg.cols, djSrcImg.rows, djSrcImg.cols, djSrcImg.rows);
			int dwMoveDis = max(FaceR.width, FaceR.height) / (rand() % 10 + 5);
			int dwCore = rand() % 180 - 90;
			FaceR.x = FaceR.x + cos(dwCore)*dwMoveDis;
			FaceR.y = FaceR.y + sin(dwCore)*dwMoveDis;
			Mat djSrcImg1 = djBacImg(FaceR).clone();

			string StoreName1 = lines.substr(0, lines.length() - 4) + "_aug_1.jpg";
			imwrite(StoreName1, djSrcImg1, compression_params);

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

			string StoreName2 = lines.substr(0, lines.length() - 4) + "_aug_2.jpg";
			imwrite(StoreName2, djSrcImg2, compression_params);					
				
			Mat djSrcImg3=djSrcImg.clone();
			vector<Mat> yuv;
			cvtColor(djSrcImg, djSrcImg3, COLOR_BGR2YCrCb);
			split(djSrcImg3, yuv);

			//equalize Y
			equalizeHist(yuv[0], yuv[0]);

			//merge channels	
			merge(yuv, djSrcImg);
			cvtColor(djSrcImg3, djSrcImg3, COLOR_YCrCb2BGR);

			for (int i = 0;i < djSrcImg.rows;i++)
			{
				for (int j = 0;j < djSrcImg.cols;j++)
				{
					if (0 == djSrcImg.at<Vec3b>(i, j)[0] && 0 == djSrcImg.at<Vec3b>(i, j)[1] && 0 == djSrcImg.at<Vec3b>(i, j)[2])
					{
						djSrcImg3.at<Vec3b>(i, j)[0] = 0;
						djSrcImg3.at<Vec3b>(i, j)[1] = 0;
						djSrcImg3.at<Vec3b>(i, j)[2] = 0;
					}
				}
			}

			string StoreName3 = lines.substr(0, lines.length() - 4) + "_aug_3.jpg";
			imwrite(StoreName3, djSrcImg3, compression_params);	
											
			// Random Move: Mean
			Mat djSrcImg4 = djSrcImg.clone();
			int dwCount = 0;
			int dwCountValue = 0;
			for (int i = 0;i < djSrcImg.rows;i++)
			{
				for (int j = 0;j < djSrcImg.cols;j++)
				{
					if (0 != djSrcImg.at<Vec3b>(i, j)[0])
					{
						dwCountValue += djSrcImg.at<Vec3b>(i, j)[0];
						dwCount++;
					}
				}
			}

			int dwRandValue = rand() % 20 + 150;
			for (int i = 0;i < djSrcImg4.rows;i++)
			{
				for (int j = 0;j < djSrcImg4.cols;j++)
				{
					if (djSrcImg4.at<Vec3b>(i, j)[0]>20)
					{
						djSrcImg4.at<Vec3b>(i, j)[0] += dwRandValue - dwCountValue / dwCount;
						djSrcImg4.at<Vec3b>(i, j)[1] += dwRandValue - dwCountValue / dwCount;
						djSrcImg4.at<Vec3b>(i, j)[2] += dwRandValue - dwCountValue / dwCount;
					}
				}
			}

			string StoreName4 = lines.substr(0, lines.length() - 4) + "_aug_4.jpg";
			imwrite(StoreName4, djSrcImg4, compression_params);
		}	
	}
	ftp.close();
}