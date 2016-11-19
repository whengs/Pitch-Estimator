#ifndef __WAVDRAW_H__
#define __WAVDRAW_H__

#include<opencv2\opencv.hpp>
#include"wavprocess.h"

using namespace cv;
using namespace std;

const int IMGMINWIDTH = 50;
const int IMGMAXWIDTH = 1280;
const int IMGMAXHEIGHT = 400;

double * sampleData(double *orgData,const int &orgLength,const int &dstLength) {
	if (orgLength <= dstLength) {
		cout << "origLength must be longer than dstLeng" << endl;
		exit(1);
	}
	double rate = orgLength / dstLength;
	double *dstData = new double[dstLength];
	for (int i = 0; i < dstLength; i++) {
		dstData[i] = orgData[int(i*rate)];
	}

	return dstData;
}

//显示wav图像，当 wavShowWidth > 32768 (short int) 时会出现问题。需进行抽样。
//draw wave image,problems may occur when wavShowWidth > 32768 (short int),so it have to be sampled
void wav2img(Mat &img, double* wavData, int wavSize, int wavStart, int wavShowWidth, int windwHeight, const char *color = "green") {
	double *sampledData;
	Scalar c(0, 255, 0);
	
	//设置颜色
	//set color
	if ((!strcmp(color, "b")) | (!strcmp(color, "blue")))
		c = Scalar(255, 0, 0);
	else if ((!strcmp(color, "r")) | (!strcmp(color, "red")))
		c = Scalar(0, 0, 255);

	//若显示长度大于32768，需进行抽样
	//sample data
	if (wavSize > 32768) {	
		sampledData = sampleData(wavData, wavSize, 32768);
		wavSize = 32768;
		wavShowWidth = 32768;
		wavData = sampledData;
	}

	//若实际长度大于显示长度，也需进行抽样
	//if length of data is larger than length to show, it has to be sampled as well
	if (wavSize > wavShowWidth) {	
		sampledData = sampleData(wavData, wavSize, wavShowWidth);
		wavSize = wavShowWidth;
		wavData = sampledData;
	}
	
	//若实际长度小于显示长度，显示长度=实际长度
	//if length of data is smaller than length to show, then assign length to show with value of length of data
	if (wavSize < wavShowWidth) {
		wavShowWidth = wavSize;
	}

	int i;
	double max = 1, min = -1;
	
	for (i = 0; i < wavSize; i++) {
		if (wavData[i] > max)
			max = wavData[i];
		if (min > wavData[i])
			min = wavData[i];
	}
	
	//cout << "max:" << max << endl;
	
	windwHeight = windwHeight > IMGMAXHEIGHT ? IMGMAXHEIGHT : windwHeight;

	int j = 0;
	Point2i prePoint, curPoint;
	if (wavShowWidth >= IMGMINWIDTH) {
		img.create(Size(wavShowWidth, windwHeight), CV_8UC3);	//short int 类型范围 -32768 ~ 32768
		img.setTo(Scalar(0, 0, 0));
		for (i = wavStart; i < wavStart + wavShowWidth; i++) {
			prePoint = Point2i(j, img.rows - (int)(windwHeight * (wavData[i] - min) / (max - min)));
			if (j)
				line(img, prePoint, curPoint, c, 2);
			curPoint = prePoint;
			j++;
		}
		//line(img, Point2i(0, int((windwHeight * (0 - min) / (max - min)))), Point2i(wavShowWidth, int((windwHeight * (0 - min) / (max - min)))), Scalar(255,255,255), 1);
		//cout << "j:" << j << endl;

		//cout << "Original image size:" << img.rows << "," << img.cols << endl;

		if (wavShowWidth > IMGMAXWIDTH) {
			//cout << "The wav is too long to show,and it will be resized to IMGMAXWIDTH" << endl;
			resize(img, img, Size(IMGMAXWIDTH, img.rows), 0, 0, CV_INTER_NN);
		}

		//cout << "Resized image size:" << img.rows << "," << img.cols << endl;

	}
	else {
		img.create(windwHeight, IMGMINWIDTH, CV_8UC3);
		img.setTo(Scalar(0, 0, 0));
		for (i = wavStart; i < wavStart + wavShowWidth; i++) {
			prePoint = Point2i(j * IMGMINWIDTH / wavShowWidth, img.rows - ((int)(windwHeight * (wavData[i] - min) / (max - min))));
			circle(img, prePoint, 3, c, CV_FILLED);
			j++;
		}
		resize(img, img, Size(IMGMINWIDTH, img.rows), 0, 0, CV_INTER_NN);
		//cout << "The wav is too short to show,and it will be resized to IMGMINWIDTH" << endl;
	}
}

#endif // !__WAVDRAW_H__
#pragma once
