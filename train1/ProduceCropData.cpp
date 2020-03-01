#include <iostream>
#include <string>
#include <fstream>


#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;


int main()
{
	//////////////////////////////////////////////////////////
	ifstream ftp("list_img.txt");		// 存储了所有图片地址的txt
	string djImgPath,djStorePath;

	Mat structure_element = getStructuringElement(MORPH_RECT, Size(3, 3));

	while (getline(ftp, djImgPath))
	{
		djStorePath = djImgPath.substr(0, djImgPath.rfind(".")) + "_crop.jpg";

		Mat dwSrcImg = imread(djImgPath);
		Mat dwDstImg = dwSrcImg.clone();

		cvtColor(dwSrcImg, dwDstImg, CV_BGR2GRAY);
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

		if (0 == djContours.size())
		{
			continue;
		}

		Rect djMaxRect(0,0,0,0);
		for (int i = 0; i < djContours.size(); i++)
		{
			djMaxRect = djMaxRect.area() > boundingRect(Mat(djContours[i])).area() ? djMaxRect : boundingRect(Mat(djContours[i]));
		}
		imwrite(djStorePath, dwSrcImg(djMaxRect));
	}
}