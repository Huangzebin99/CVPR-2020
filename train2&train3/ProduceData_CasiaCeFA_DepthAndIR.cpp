#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include <direct.h>
#include <io.h>
//#include <vector>
//#include <iomanip>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

//#define TRAIN2
#define TRAIN3
#define TEST

// 为图像添加亮斑
void gAddBrightPoint(const Mat& djSrcImg, Mat& djDstImg, const Mat& djBackImg)
{
	djDstImg = djSrcImg.clone();

	//int Ratio = rand() % 5;
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
			circle(djDstImg, Point(dwTempCols, dwTempRows), dwAllCols / (rand() % 20 + 40), Scalar(220 + rand() % 36), -1);
		}
		else
		{
			i--;
		}
	}
}

//亮度判断
int gJudgeLight(const Mat& djSrc)
{
	Mat Mask, Back;
	cvtColor(djSrc, Mask, CV_BGR2GRAY);
	cvtColor(djSrc, Back, CV_BGR2GRAY);

	threshold(Mask, Mask, 245, 255, CV_THRESH_BINARY);
	threshold(Back, Back, 0, 255, CV_THRESH_BINARY);

	int CountB = 0, CountM = 0;
	for (int i = 0;i < Mask.rows;i++)
	{
		for (int j = 0;j < Mask.cols;j++)
		{
			if (Back.at<uchar>(i, j))
			{
				CountM++;
				if (Mask.at<uchar>(i, j))
					CountB++;
			}
		}
	}

	int CountMax = 0, CountMin = 9999;

	if (CountB && CountM)
	{
		CountMax = CountB * 100 / CountM > CountMax ? CountB * 100 / CountM : CountMax;
		CountMin = CountB * 100 / CountM < CountMin ? CountB * 100 / CountM : CountMin;

		if (CountB * 100 / CountM > 55)
		{
			return 1;
		}
	}

	return 0;
}

// 图像校正
void gCorrect(Mat& djSrcDepthImg)
{
	/*Mat djBacImg(djSrcDepthImg.size() * 3, CV_8UC3, Scalar(0, 0, 0));
	djSrcDepthImg.copyTo(djBacImg(Rect(djSrcDepthImg.cols, djSrcDepthImg.rows, djSrcDepthImg.cols, djSrcDepthImg.rows)));
	Mat djBacTempImg;
	cvtColor(djBacImg, djBacTempImg, CV_BGR2GRAY);

	threshold(djBacTempImg, djBacTempImg, 0, 255, CV_THRESH_BINARY);

	vector<vector<Point>> djContours;
	vector<Vec4i> djHierarchy;
	findContours(djBacTempImg, djContours, djHierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	RotatedRect Maxrect;
	Maxrect.size.height = 0;
	Maxrect.size.width = 0;

	for (int i = 0;i < djContours.size();i++)
	{
		Maxrect = Maxrect.size.height*Maxrect.size.width>minAreaRect(djContours[i]).size.height*minAreaRect(djContours[i]).size.width ? Maxrect : minAreaRect(djContours[i]);
	}

	Point2f pt(djBacImg.cols / 2., djBacImg.rows / 2.);
	double angle = Maxrect.size.width < Maxrect.size.height ? Maxrect.angle : -(abs(Maxrect.angle) - 90);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(djBacImg, djBacImg, r, djBacImg.size());

	Mat djBacImgsss = djBacImg.clone();
	cvtColor(djBacImgsss, djBacImgsss, CV_BGR2GRAY);
	threshold(djBacImgsss, djBacImgsss, 0, 255, CV_THRESH_BINARY);

	vector<vector<Point>> djContoursT;
	vector<Vec4i> djHierarchyT;
	findContours(djBacImgsss, djContoursT, djHierarchyT, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Rect djMaxRectT(0, 0, 0, 0);
	for (int i = 0; i < djContoursT.size(); i++)
	{
		djMaxRectT = djMaxRectT.area() > boundingRect(Mat(djContoursT[i])).area() ? djMaxRectT : boundingRect(Mat(djContoursT[i]));
	}
	djSrcDepthImg = djBacImg(djMaxRectT);*/

	Mat structure_element = getStructuringElement(MORPH_RECT, Size(3, 3));
	Mat dwSrcImg = djSrcDepthImg.clone();
	Mat dwDstImg = dwSrcImg.clone();

	cvtColor(dwDstImg, dwDstImg, CV_BGR2GRAY);
	cv::threshold(dwDstImg, dwDstImg, 1, 255, THRESH_BINARY);

	dilate(dwDstImg, dwDstImg, structure_element); //膨胀

	for (int i = 0;i < dwDstImg.rows;i++)
	{
		for (int j = 0;j < dwDstImg.cols;j++)
		{
			dwDstImg.at<uchar>(i, j) = dwDstImg.at<uchar>(i, j)>0 ? 255 : 0;
		}
	}

	vector<vector<Point>> djContours;
	vector<Vec4i> djHierarchy;
	findContours(dwDstImg, djContours, djHierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Rect djMaxRect(0, 0, 0, 0);
	for (int i = 0; i < djContours.size(); i++)
	{
		djMaxRect = djMaxRect.area() > boundingRect(Mat(djContours[i])).area() ? djMaxRect : boundingRect(Mat(djContours[i]));
	}
	djSrcDepthImg = dwSrcImg(djMaxRect).clone();
}


int main()
{
	// Augmentation Depth
	ifstream ftp("./train/train_3/4@3_img_ir.txt");	// 训练数据，仅包含每个id下”ir“子文件夹下的所有图片和”profile“子文件夹下的所有图片

	string lines, linesIR;

	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0);
	compression_params.push_back(IMWRITE_PNG_STRATEGY);
	compression_params.push_back(IMWRITE_PNG_STRATEGY_DEFAULT);

	while (getline(ftp, lines))
	{
		cout << lines << endl;
		if (lines.find("ir") != string::npos)
		{
			string linesIR = lines;
			string linesDepth = lines.substr(0, lines.find("ir") - 1) + "/depth" + lines.substr(lines.find("ir") + 2, lines.length());

			Mat djSrcIRImg = imread(lines);
			Mat djSrcDepthImg = imread(linesDepth);

			if (1 == gJudgeLight(djSrcDepthImg))	//这里判断深度图亮度，大于一定亮度的直接当成真人
			{
				gCorrect(djSrcIRImg);
				string StoreName2 = linesIR.substr(0, linesIR.length() - 4) + "_crop.jpg";
				imwrite(StoreName2, djSrcIRImg, compression_params);
				continue;
			}

			// 图像旋转校正
			gCorrect(djSrcDepthImg);
			gCorrect(djSrcIRImg);		

			// 深度图像处理
			cvtColor(djSrcDepthImg, djSrcDepthImg, CV_BGR2GRAY);

			int MaxV = 0;
			int MinV = 999;

			for (int i = 0;i < djSrcDepthImg.rows;i++)
			{
				for (int j = 0;j < djSrcDepthImg.cols;j++)
				{
					MaxV = djSrcDepthImg.at<uchar>(i, j)>MaxV ? djSrcDepthImg.at<uchar>(i, j) : MaxV;
				}
			}

#ifdef TRAIN2
			// version train 2
			MinV = MaxV - 100>0 ? MaxV - 100 : 0;
#endif

#ifdef TRAIN3
			// version train 3
			MinV = MaxV - 150>0 ? MaxV - 150 : 0;
#endif

			for (int i = 0;i < djSrcDepthImg.rows;i++)
			{
				for (int j = 0;j < djSrcDepthImg.cols;j++)
				{
					if (djSrcDepthImg.at<uchar>(i, j) < MinV)
						djSrcDepthImg.at<uchar>(i, j) = 0;
					else if (djSrcDepthImg.at<uchar>(i, j)>20)
#ifdef TRAIN2
						// version train2
						djSrcDepthImg.at<uchar>(i, j) = saturate_cast<uchar>((djSrcDepthImg.at<uchar>(i, j) - MinV) * 55 / (MaxV - MinV + 1) + 200);
#endif

#ifdef TRAIN3
						// version train3
						djSrcDepthImg.at<uchar>(i, j) = saturate_cast<uchar>((djSrcDepthImg.at<uchar>(i, j) - MinV) * 100 / (MaxV - MinV + 1) + 155);
#endif
				}
			}
			cvtColor(djSrcDepthImg, djSrcDepthImg, CV_GRAY2BGR);

			//imshow("2", djSrcDepthImg);
			//waitKey();

			string StoreName1 = linesDepth.substr(0, linesDepth.length() - 4) + "_crop.jpg";
			imwrite(StoreName1, djSrcDepthImg, compression_params);

			string StoreName2 = linesIR.substr(0, linesIR.length() - 4) + "_crop.jpg";
			imwrite(StoreName2, djSrcIRImg, compression_params);			
		}
		else if (lines.find("profile") != string::npos)
		{
#ifdef TEST
			continue;
#endif
			// 使用 RGB图像转 IR图生成面具攻击
			Mat djSrcImg = imread(lines);
			gCorrect(djSrcImg);

			cvtColor(djSrcImg, djSrcImg, CV_BGR2GRAY);

			// get background area
			Mat djMaskBack;
			threshold(djSrcImg, djMaskBack, 0, 255, CV_THRESH_BINARY);

			// 双边滤波磨皮
			int dwKernel = rand() % 10 + 5;	
			Mat djTemp;
			bilateralFilter(djSrcImg, djTemp, dwKernel, dwKernel * 5, dwKernel / 2);
			for (int i = 0;i < djSrcImg.rows;i++)
			{
				for (int j = 0;j < djSrcImg.cols;j++)
				{
					if (djSrcImg.at<uchar>(i, j)>20)
						djSrcImg.at<uchar>(i, j) = djTemp.at<uchar>(i, j);
				}
			}

			// 随机判断是否进行模糊
			if (0 == rand() % 2)	
			{
				//随机判断是否添加亮斑
				if (0 == rand() % 2)
				{
					gAddBrightPoint(djSrcImg, djSrcImg, djMaskBack);
				}
			}
			else
			{
				if (0 == rand() % 2)
				{
					gAddBrightPoint(djSrcImg, djSrcImg, djMaskBack);

					int dwKerSize = rand() % 25 + 5;
					dwKerSize = dwKerSize % 2 == 0 ? dwKerSize - 1 : dwKerSize;
					int dwStd = rand() % 25 + 5;
					GaussianBlur(djSrcImg, djSrcImg, Size(dwKerSize, dwKerSize), dwStd, dwStd);
				}
				else
				{
					// store blur
					int dwKerSize = rand() % 25 + 5;
					dwKerSize = dwKerSize % 2 == 0 ? dwKerSize - 1 : dwKerSize;
					int dwStd = rand() % 25 + 5;
					GaussianBlur(djSrcImg, djSrcImg, Size(dwKerSize, dwKerSize), dwStd, dwStd);
				}
			}

			if (lines.find("real") != string::npos)
			{
				string StoreName = lines.substr(0, lines.length() - 4) + "_aug_1.jpg";
				StoreName = StoreName.substr(0, StoreName.find("attack") - 1) + "/attack_supply/" + StoreName.substr(StoreName.find("attack") + 7, StoreName.length());
				cvtColor(djSrcImg, djSrcImg, CV_GRAY2BGR);
				imwrite(StoreName, djSrcImg, compression_params);
			}
			else
			{
				string StoreName = lines.substr(0, lines.length() - 4) + "_aug_2.jpg";
				StoreName = StoreName.substr(0, StoreName.find("profile") - 1) + "/ir/" + StoreName.substr(StoreName.find("profile") + 8, StoreName.length());
				cvtColor(djSrcImg, djSrcImg, CV_GRAY2BGR);
				imwrite(StoreName, djSrcImg, compression_params);
			}			
		}
	}
	ftp.close();
}
