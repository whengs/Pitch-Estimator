#pragma once
#ifndef __FRAME_H__
#define __FRAME_H__

#include<iostream>
#include<fstream>
#include<opencv2\opencv.hpp>
#include<cmath>
#include"wavprocess.h"
#include"wavdraw.h"

using namespace cv;

//SamplePerSec:44100
//const int FRAMELENGTH = 1050;
//const int FRAMESHIFT = 350;

int FRAMELENGTH;
int FRAMESHIFT;

class frame {	
public:
	double *frameData;
	double *acfData;
	frame(const int &SamplePerSec);
	~frame();	//destructor
	void getFrameData(wavdata &dt, const int &Channel, const int &MemSize, const int &shiftTimes);
	void calACF();	//cclculate ACF of one frame
	int calZCR();	//calculate zero-crossing rate in one frame
	int calMean();	//calculate DC bias
};

frame::frame(const int &SamplePerSec) {
	FRAMELENGTH = int(SamplePerSec / 42);	//42 frames per second
	FRAMESHIFT = int(FRAMELENGTH / 3);	//frame shift is one third of the frame length
	frameData = new double[FRAMELENGTH];
	acfData = new double[FRAMELENGTH];
}

frame::~frame() {
	delete[]frameData;
	delete[]acfData;
}

void frame::getFrameData(wavdata &dt, const int &Channel, const int &MemSize, const int &shiftTimes) {	//获得一帧数据，shiftTimes为帧移次数
	
	if (MemSize == (shiftTimes - 1) * FRAMESHIFT + FRAMELENGTH)
		exit(0);

	if (shiftTimes > int((MemSize - FRAMELENGTH) / FRAMESHIFT + 1)) {
		cout << "ERROR INPUT I" << endl;
		exit(1);
	}

	else if (shiftTimes < int((MemSize - FRAMELENGTH) / FRAMESHIFT) + 1) {
		if (Channel == 1)
			for (int j = 0; j < FRAMELENGTH; j++)
				frameData[j] = dt.ChanOneStd[FRAMESHIFT * shiftTimes + j];
		else if (Channel == 2)
			for (int j = 0; j < FRAMELENGTH; j++)
				frameData[j] = dt.ChanTwoStd[FRAMESHIFT * shiftTimes + j];
	}
	else if (shiftTimes == int((MemSize - FRAMELENGTH) / FRAMESHIFT + 1)) {
		
		if (Channel == 1) {
			int j;
			for (j = 0; j < MemSize - FRAMESHIFT*shiftTimes; j++)
				frameData[j] = dt.ChanOneStd[FRAMESHIFT * shiftTimes + j];
			for (int k = j; k < FRAMELENGTH; k++)
				frameData[k] = 0;

			//_ASSERTE(_CrtCheckMemory());

		}
		else if (Channel == 2) {
			int j;
			for (j = 0; j < MemSize - FRAMESHIFT*shiftTimes; j++)
				frameData[j] = dt.ChanTwoStd[FRAMESHIFT * shiftTimes + j];
			for (int k = j; k < FRAMELENGTH; k++)
				frameData[k] = 0;

			//_ASSERTE(_CrtCheckMemory());

		}
	}
}

void frame::calACF() {	//calculate autocorrelation: r(m)= ((N-1) sum n=m) x(n-m)*x(n)
	double sum = 0;
	for (int i = 0; i < FRAMELENGTH; i++) {
		acfData[i] = 0;
		for (int j = i; j < FRAMELENGTH - 1; j++) {
			sum += frameData[j] * frameData[j - i];
		}
		acfData[i] = sum;
		sum = 0;
	}
}

template <typename Elemtype>
int sgn(const Elemtype &num);

int frame::calZCR() {
	int zn = 0;
	for (int i = 1; i < FRAMELENGTH; i++) {
		zn += abs(sgn(frameData[i]) - sgn(frameData[i - 1]));
	}
	zn = int(zn / 2);
	return zn;
}

int frame::calMean() {
	double mean = 0;
	for (int i = 0; i < FRAMELENGTH; i++)
		mean += frameData[i];
	mean /= FRAMELENGTH;
	return mean;
}

template <typename Elemtype>
int sgn(const Elemtype &num) {
	if (num >= 0)
		return 1;
	else if (num < 0)
		return -1;
}

#endif // !__FRAME_H__

