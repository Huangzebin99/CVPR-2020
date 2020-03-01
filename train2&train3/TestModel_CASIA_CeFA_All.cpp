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

//����ͼ���ֵ����
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

// ͨ�����Թ���һ���ֹ�������
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

	// ���� ͼ���ֵ�ͷ���
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

	//�ж������Ӿ����Լ����߸���
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
		// ÿ���ļ�����ʹ�ýű�����һ��������С�*_crop.jpg��ͼƬ��txt����ȡ��txt
		DepthPathString = DepthPathOrg + "/depth/list_img_list.txt";
		IRPathString= DepthPathOrg + "/ir/list_img_list.txt";

		int dwDepthFileLabel = 0;	// ����㷨������ 0��ʾ���� 1��ʾ����
		int dwFileLabel = 0;  // ���ս��

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

			// ����ʹ��caffe �� depthģ��ǰ�򣬸���ģ���ж�ͼƬΪ���˵�score
			caffe_depth_process		

			if (score < dwDepththresh)
				dwDepthCountAttack++;
		}
		FtpDepth.close();

		//һƱ����ƣ���Ϊ0ֱ���ж�Ϊ������0ʱû�����ɶ�Ӧ��crop���ݣ���δ��������Ҫ��δ����ԭ���/
		// ProduceData_CasiaCeFA_DepthAndIR�еġ�gJudgeLight������
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

			if (dwDepthFileLabel == 0)	// �������ж�Ϊ�����������ս��Ϊ�������������IR�ж�
			{
				dwFileLabel = 1;
			}
			else
			{
				// IR�����ж�
				ifstream FtpIR(IRPathString);
				int dwIRCountNum = 0, dwIRCountAttack = 0;
				while (getline(FtpIR, IRImgPath))
				{
					Mat IRImg = imread(IRImgPath);
					if (IRImg.empty())
						continue;

					dwIRCountNum++;
					if (0 == gJudgeStrategy(IRImg))		// ���˲��ֹ���/����/���аߵ��IRͼ
					{
						dwIRCountAttack++;
						continue;
					}

					resize(IRImg, IRImg, Size(DEPTH_INPUT_WIDTH, DEPTH_INPUT_HEIGHT));

					// ����ʹ��caffe �� irģ��ǰ�򣬸���ģ���ж�ͼƬΪ���˵�score
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