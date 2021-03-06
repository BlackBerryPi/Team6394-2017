#include "opencv2/highgui/highgui.hpp"

#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

#include <stdio.h>

#include <stdlib.h>



using namespace cv;

using namespace std;



//反光条大小

int AreaMin = 800;

int CamBuff = 6;


//反光条长宽比

int RefleMaxRatio = 4.0;

int RefleMinRatio = 2.0;



//反光条间距-宽度比

int RefleInterRatio = 2.0;

const int Max_RefleInterRatio = 5.0;



Mat src; Mat src_gray;

int thresh = 150;

int max_thresh = 255;

RNG rng(12345);



/// 函数声明

void thresh_callback(int, void*);

double similar(int a, int b) {

	return abs(double(a) - double(b)) / (double(a) + double(b));

}



/** @主函数 */

int main(int argc, char** argv)
{
	char* source_window = "Source";

	namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	
	///开启摄像头
	VideoCapture cap(0); // open the video camera no. 0

   	if (!cap.isOpened()){

        	return -1;

    }
	
	while(1)
	{
		for(int i=0; i<CamBuff; i++)
		{
			cap.read(src);
		}
		
		resize(src, src, Size(), 0.5, 0.5);
		
		
		/// 转化成灰度图像并进行平滑
		
		cvtColor(src, src_gray, CV_BGR2GRAY);
		
		//blur(src_gray, src_gray, Size(3, 3));
		
		///显示图像
		imshow(source_window, src);
	
		/// 转化成灰度图像并进行平滑
	
		cvtColor(src, src_gray, CV_BGR2GRAY);
	
		//blur(src_gray, src_gray, Size(3, 3));
	
	
		imshow(source_window, src);
	
	
	
		createTrackbar(" Threshold:", "Source", &thresh, max_thresh, thresh_callback);
	
		createTrackbar(" 反光条间距:", "Source", &RefleInterRatio, Max_RefleInterRatio, thresh_callback);
	
		thresh_callback(0, 0);
	
		waitKey(0);
	}
	
	return(0);

}



/** @thresh_callback 函数 */

void thresh_callback(int, void*)

{

	Mat threshold_output;



	vector<vector<Point> > contours;

	vector<Vec4i> hierarchy;

	/// 使用Threshold检测边缘

	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);

	namedWindow("ThresholdView");

	imshow("ThresholdView", threshold_output);

	/// 找到轮廓

	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));



	/// 多边形逼近轮廓 + 获取矩形和圆形边界框

	vector<vector<Point> > contours_poly(contours.size());

	vector<Rect> boundRect(contours.size());

	vector<Point2f>center(contours.size());

	vector<float>radius(contours.size());

	vector<double>area(contours.size());

	vector<bool>RectFlag(contours.size());

	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

	double TmpArea;

	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);

	int i, j;

	double TmpRatio;

	for (i = 0; i < contours.size(); i++)

	{

		approxPolyDP(Mat(contours[i]), contours_poly[i], 10, true);

		//拟合赋值给contours_poly

		boundRect[i] = boundingRect(Mat(contours_poly[i]));

		//拟合矩形

		TmpRatio = (double(boundRect[i].height) / double(boundRect[i].width));

		TmpArea = (double(boundRect[i].height) * double(boundRect[i].width));

		if ((TmpRatio <= RefleMaxRatio) && (TmpRatio >= RefleMinRatio) && (TmpArea >= AreaMin)) {

			cout << TmpRatio << endl;

			RectFlag[i] = true;

			area[i] = TmpArea;

			rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 1, 8, 0);

		}

		else RectFlag[i] = false;



		//minEnclosingCircle(contours_poly[i], center[i], radius[i]);

		//拟合圆形

	}



	//寻找荧光条

	double MatchScore = 2;

	double TmpScore;

	int area1, area2, h1, h2;

	int r1 = -1;

	int r2 = -1;



	for (i = 0; i < contours.size(); i++)

	{

		for (j = 0; j < contours.size(); j++)

		{

			if ((RectFlag[i] == true) && (RectFlag[j] == true) && (i<j)) {

				//TmpScore = abs((abs(boundRect[i].x - boundRect[j].x) / (boundRect[i].width + boundRect[j].width)) / RefleInterRatio - 1);

				area1 = area[i];

				area2 = area[j];

				h1 = boundRect[i].y;

				h2 = boundRect[j].y;

				TmpScore = similar(area1, area2);

				TmpScore = TmpScore*similar(h1, h2);

				cout << "Score:" << TmpScore << endl;

				if (TmpScore < MatchScore) {

					MatchScore = TmpScore;

					r1 = i;

					r2 = j;

				}

			}

		}

	}

	namedWindow("Contours", CV_WINDOW_AUTOSIZE);

	if (r1 != -1 && r2 != -1) {

		area1 = boundRect[r1].width *boundRect[r1].height;

		area2 = boundRect[r2].width *boundRect[r2].height;





		/// 画多边形轮廓 + 包围的矩形框 + 圆形框

		color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		rectangle(drawing, boundRect[r1].tl(), boundRect[r1].br(), color, 3, 8, 0);

		rectangle(drawing, boundRect[r2].tl(), boundRect[r2].br(), color, 3, 8, 0);

		}



		/// 显示在一个窗口

		imshow("Contours", drawing);
	
}
