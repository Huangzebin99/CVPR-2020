#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include <time.h>
#include <direct.h>

//#include <vector>
//#include <iomanip>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

#define DEPTH_INPUT_WIDTH  114
#define DEPTH_INPUT_HEIGHT 168
#define IR_INPUT_WIDTH  114
#define IR_INPUT_HEIGHT 168

//计算图像均值方差
void gGetMeanAndStd(Mat& img, int& MeanValue, int& StdValue)
{
	double Sum = 0;
	int CountNum = 0;
	for (int i = 0;i < img.rows;i++)
	{
		for (int j = 0;j < img.cols;j++)
		{
			if (img.at<uchar>(i, j))
			{
				Sum += img.at<uchar>(i, j);
				CountNum++;
			}
		}
	}

	MeanValue = Sum / CountNum;

	int StdSumValue = 0;
	for (int i = 0;i < img.rows;i++)
	{
		for (int j = 0;j < img.cols;j++)
		{
			if (img.at<uchar>(i, j))
			{
				StdSumValue += pow(img.at<uchar>(i, j) - MeanValue, 2);
			}
		}
	}

	StdValue = StdSumValue / CountNum;
}

// 通过策略过滤一部分攻击数据
int gJudgeStrategy(Mat& djSrc)
{
	Mat structure_element = getStructuringElement(MORPH_RECT, Size(3, 3));

	Mat Depth1;
	cvtColor(djSrc, Depth1, CV_BGR2GRAY);

	// Judge
	int MaxV = 0;
	int MaxList[256];
	memset(MaxList, 0, sizeof(int) * 256);

	for (int i = 0;i < Depth1.rows;i++)
	{
		for (int j = 0;j < Depth1.cols;j++)
		{
			if (Depth1.at<uchar>(i, j) >= 40)
				MaxList[Depth1.at<uchar>(i, j)]++;
		}
	}

	// 计算 图像均值和方差
	int MeanValue = 0, StdValue = 0;
	gGetMeanAndStd(Depth1, MeanValue, StdValue);

	if (StdValue>800 || MeanValue>200)
	{
		return 0;
	}

	for (int i = 0;i < 255;i++)
	{
		if (MaxList[255 - i] >= 10)
		{
			MaxV = 255 - i - 40;
			break;
		}
	}

	threshold(Depth1, Depth1, MaxV, 255, CV_THRESH_BINARY);

	dilate(Depth1, Depth1, structure_element);
	for (int i = 0;i < Depth1.rows;i++)
	{
		for (int j = 0;j < Depth1.cols;j++)
		{
			Depth1.at<uchar>(i, j) = Depth1.at<uchar>(i, j)>0 ? Depth1.at<uchar>(i, j) : 0;
		}
	}
	//imshow("1", Depth1);

	vector<vector<Point>> djContoursT;
	vector<Vec4i> djHierarchyT;
	findContours(Depth1, djContoursT, djHierarchyT, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	Rect djMaxRectT(0, 0, 0, 0);
	for (int i = 0; i < djContoursT.size(); i++)
	{
		djMaxRectT = djMaxRectT.area() > boundingRect(Mat(djContoursT[i])).area() ? djMaxRectT : boundingRect(Mat(djContoursT[i]));
	}

	//判断最大外接矩形以及亮斑个数
	if (djContoursT.size() > 10 || djMaxRectT.area() < Depth1.rows*Depth1.cols * 7 / 10)
	{
		return 0;
	}

	return 1;
}

int main()
{
	// test image
	ifstream ftp("4@3_sum.txt");
	ofstream ResultFtp("4@3_sum_result.txt");
	string DepthPathOrg, DepthPath, DepthPathString, IRPathString;

	double dwDepththresh = 0.5;
	double dwIRthresh = 0.5;

	Mat structure_element = getStructuringElement(MORPH_RECT, Size(3, 3)); 

	while (getline(ftp, DepthPathOrg))
	{
		// 每个文件夹下使用脚本生成一个存放所有“*_crop.jpg”图片的txt，读取该txt
		DepthPathString = DepthPathOrg + "/depth/list_img_list.txt";
		IRPathString= DepthPathOrg + "/ir/list_img_list.txt";

		int dwDepthFileLabel = 0;	// 深度算法处理结果 0表示攻击 1表示真人
		int dwFileLabel = 0;  // 最终结果

		// process
		ifstream FtpDepth(DepthPathString);

		int dwDepthCountNum = 0, dwDepthCountAttack = 0;
		string DepthImgPath,IRImgPath;

		while (getline(FtpDepth, DepthImgPath))
		{
			Mat Depth = imread(DepthImgPath);
			if (Depth.empty())
			{
				continue;
			}
			dwDepthCountNum++;

			resize(Depth, Depth, Size(DEPTH_INPUT_WIDTH, DEPTH_INPUT_HEIGHT));

			// 这里使用caffe 跑 depth模型前向，给出模型判断图片为真人的score
			caffe_depth_process		

			if (score < dwDepththresh)
				dwDepthCountAttack++;
		}
		FtpDepth.close();

		//一票否决制，如为0直接判断为攻击（0时没有生成对应的crop数据，即未满足亮度要求，未生成原因见/
		// ProduceData_CasiaCeFA_DepthAndIR中的“gJudgeLight”函数
		if (0 == dwDepthCountNum)	
		{
			dwFileLabel = 1;
		}
		else
		{
			if (dwDepthCountNum - dwDepthCountAttack >= 10)		
				dwFileLabel = 1;
			else if (dwDepthCountAttack > dwDepthCountNum / 2)
				dwDepthFileLabel = 0;
			else
				dwDepthFileLabel = 1;

			if (dwDepthFileLabel == 0)	// 如果深度判断为攻击，则最终结果为攻击，否则进行IR判断
			{
				dwFileLabel = 1;
			}
			else
			{
				// IR活体判断
				ifstream FtpIR(IRPathString);
				int dwIRCountNum = 0, dwIRCountAttack = 0;
				while (getline(FtpIR, IRImgPath))
				{
					Mat IRImg = imread(IRImgPath);
					if (IRImg.empty())
						continue;

					dwIRCountNum++;
					if (0 == gJudgeStrategy(IRImg))		// 过滤部分过亮/过暗/带有斑点的IR图
					{
						dwIRCountAttack++;
						continue;
					}

					resize(IRImg, IRImg, Size(DEPTH_INPUT_WIDTH, DEPTH_INPUT_HEIGHT));

					// 这里使用caffe 跑 ir模型前向，给出模型判断图片为真人的score
					caffe_ir_process
					
					if (score < dwIRthresh)
						dwIRCountAttack++;
				}
				FtpIR.close();

				if (dwIRCountAttack > dwIRCountNum / 2)
					dwFileLabel = 0;
				else
					dwFileLabel = 1;
			}			
		}

		string storeLines = DepthPathOrg + " " + to_string(dwFileLabel);
		ResultFtp << storeLines << endl;
	}
	return 0;
}