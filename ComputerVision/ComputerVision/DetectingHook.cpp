#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace cv;
using namespace std;

//��������С
int AreaMax = 1000;

//�����������
int RefleMaxRatio = 4.0;
int RefleMinRatio = 2.0;

//���������-��ȱ�
int RefleInterRatio = 2.0;
const int Max_RefleInterRatio = 5.0;

Mat src; Mat src_gray;
int thresh = 150;
int max_thresh = 255;
RNG rng(12345);

/// ��������
void thresh_callback(int, void*);
double similar(int a, int b) {
	return abs(double(a) - double(b)) / (double(a) + double(b));
}

/** @������ */
int main(int argc, char** argv)
{
	/// ����ԭͼ��, ����3ͨ��ͼ��
	src = imread("1.jpg", 1);

	/// ת���ɻҶ�ͼ�񲢽���ƽ��
	cvtColor(src, src_gray, CV_BGR2GRAY);
	//blur(src_gray, src_gray, Size(3, 3));

	/// ��������
	char* source_window = "Source";
	namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	imshow(source_window, src);

	createTrackbar(" Threshold:", "Source", &thresh, max_thresh, thresh_callback);
	createTrackbar(" ���������:", "Source", &RefleInterRatio, Max_RefleInterRatio, thresh_callback);
	thresh_callback(0, 0);

	waitKey(0);
	return(0);
}

/** @thresh_callback ���� */
void thresh_callback(int, void*)
{
	Mat threshold_output;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	/// ʹ��Threshold����Ե
	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);
	namedWindow("ThresholdView");
	imshow("ThresholdView", threshold_output);
	/// �ҵ�����
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// ����αƽ����� + ��ȡ���κ�Բ�α߽��
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
		//��ϸ�ֵ��contours_poly
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
		//��Ͼ���
		TmpRatio = (double(boundRect[i].height) / double(boundRect[i].width));
		TmpArea = (double(boundRect[i].height) * double(boundRect[i].width));
		if ((TmpRatio <= RefleMaxRatio) && (TmpRatio >= RefleMinRatio) && (TmpArea >= AreaMax)) {
			cout << TmpRatio << endl;
			RectFlag[i] = true;
			area[i] = TmpArea;
			rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 1, 8, 0);
		}
		else RectFlag[i] = false;

		//minEnclosingCircle(contours_poly[i], center[i], radius[i]);
		//���Բ��
	}

	//Ѱ��ӫ����
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


		/// ����������� + ��Χ�ľ��ο� + Բ�ο�
		color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		rectangle(drawing, boundRect[r1].tl(), boundRect[r1].br(), color, 3, 8, 0);
		rectangle(drawing, boundRect[r2].tl(), boundRect[r2].br(), color, 3, 8, 0);
		/*for (int i = 0; i< contours.size(); i++)
		{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
		rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
		circle(drawing, center[i], (int)radius[i], color, 2, 8, 0);
		}*/

		/// ��ʾ��һ������
		imshow("Contours", drawing);
	}




}