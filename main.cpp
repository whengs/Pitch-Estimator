#include<iostream>
#include<fstream>
#include<opencv2/opencv.hpp>
#include<sstream>
#include<cmath>
#include"wavprocess.h"
#include"wavdraw.h"
#include"frame.h"
#include"filt.h"

using namespace cv;

int main()
{
	const int mainWindowHeight = 800;
	const int wavWindowHeight = 160;
	const int frameWindowWidth = 300;
	const int frameWindowHeight = 160;	
	string wavfile = "d:\\brian.wav";
	string outfile = "d:\\brian.txt";

	Scalar white(255, 255, 255);
	Scalar black(0, 0, 0);
	Scalar red(0, 0, 255);

	//open .wav file
	fstream f;
	f.open(wavfile, ios::in | ios::binary);
	if (f.fail()) {
		cout << "Failed to open the file" << endl;
		exit(1);
	}

	//read the headerfile of .wav file
	headerFile hf;
	hf.read(f);
	hf.show();
	//cout << "MemSize:" << hf.MemSize() << endl;

	//read data block form .wav file
	wavdata data(hf.Channel(),hf.BitsPerSample(),hf.MemSize());
	if (data.read(f, hf.Channel(),hf.BitsPerSample(),hf.MemSize()))
		cout << "wav read OK!" << endl;
	f.close();

	//write data to .txt file
	fstream outFile;
	outFile.open(outfile, ios::out);
	if (outFile.fail()) {
		cout << "Fail to write the txt" << endl;
		exit(1);
	}
	data.writeTxt(outFile, hf.Channel(), hf.BitsPerSample(), hf.MemSize());
	outFile.close();

	//standardize data to value range of [-1,1]
	data.standardize(hf.Channel(),hf.BitsPerSample(),hf.MemSize());

	//initialize Mat variables
	Mat test;
	test.create(Size(IMGMAXWIDTH, mainWindowHeight), CV_8UC3);
	test.setTo(black);
	
	Mat imgWavChanOne;
	wav2img(imgWavChanOne,data.ChanOneStd, hf.MemSize(), 0, IMGMAXWIDTH, wavWindowHeight);
	putText(imgWavChanOne, "Channel 1", Point2i(0,30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
	line(imgWavChanOne, Point2i(0, int(imgWavChanOne.rows*0.5)), Point2i(imgWavChanOne.cols, int(imgWavChanOne.rows*0.5)), white);	//»­ºáÖá

	Mat imgLpfChanOne;
	imgLpfChanOne.create(Size(frameWindowWidth, frameWindowHeight), CV_8UC3);
	imgLpfChanOne.setTo(black);

	Mat imgCutOffChanOne;
	imgCutOffChanOne.create(Size(frameWindowWidth, frameWindowHeight), CV_8UC3);
	imgCutOffChanOne.setTo(black);

	Mat imgAcfChanOne;
	imgAcfChanOne.create(Size(frameWindowWidth, frameWindowHeight), CV_8UC3);
	imgAcfChanOne.setTo(black);

	Mat imgFmChanOne;
	//show images in one window
	imgWavChanOne.copyTo(test(Rect(Point2i(0, 0), Size(imgWavChanOne.cols, imgWavChanOne.rows))));

	Mat imgWavChanTwo;
	Mat imgLpfChanTwo;
	Mat  imgCutOffChanTwo;
	Mat imgAcfChanTwo;
	Mat imgFmChanTwo;

	if (hf.Channel() == 2) {

		wav2img(imgWavChanTwo, data.ChanTwoStd, hf.MemSize(), 0, IMGMAXWIDTH, wavWindowHeight, "blue");
		putText(imgWavChanTwo, "Channel 2", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
		line(imgWavChanTwo, Point2i(0, int(imgWavChanTwo.rows*0.5)), Point2i(imgWavChanTwo.cols, int(imgWavChanTwo.rows*0.5)), white);

		imgLpfChanTwo.create(Size(frameWindowWidth, frameWindowHeight), CV_8UC3);
		imgLpfChanTwo.setTo(black);

		imgCutOffChanTwo.create(Size(frameWindowWidth, frameWindowHeight), CV_8UC3);
		imgCutOffChanTwo.setTo(black);

		imgAcfChanTwo.create(Size(frameWindowWidth, frameWindowHeight), CV_8UC3);
		imgAcfChanTwo.setTo(black);

		imgWavChanTwo.copyTo(test(Rect(Rect(Point2i(0, wavWindowHeight), Size(imgWavChanOne.cols, imgWavChanOne.rows)))));
	}

	imshow("Main", test);

	frame frame(hf.SamplePerSec());

	//defint FIR low pass filter with cutoff frequency of 900 Hz
	Filter filter(LPF, 20, 44100, 900);	

	double energyThreshold[3] = { 0,0,0 };	//short time energy threshold
	double Emax[3] = { 0,0,0 };
	double Emin[3] = { 0,0,0 };
	double Eint[3] = { 0,0,0 };
	double Erms = 0;	//root mean square energy
	double lambda = 0;
	double km = 1.001;
	const int k1 = 40;
	const int k2 = 20;
	const int k3 = 2;
	const int ZCRThreshold = int(FRAMELENGTH * 1000 * 2.5 / hf.SamplePerSec());	//zero-crossing rate threshold : 59 times in one frame (24ms)
																				//check out SUVING: AUTOMATIC SILENCE /UNVOICED/VOICED CLASSIFICATION OF SPEECH	by Mark Greenwood, Andrew Kinghorn for more information
	cout << "ZCRThreshold:" << ZCRThreshold << endl;
	bool isVoiced = false;

	double *EnergyChOne = new double[int((hf.MemSize() - FRAMELENGTH) / FRAMESHIFT) + 1];
	double *EnergyChTwo = new double[int((hf.MemSize() - FRAMELENGTH) / FRAMESHIFT) + 1];
	for (int i = 0; i <= int((hf.MemSize() - FRAMELENGTH) / FRAMESHIFT) + 1; i++) {
		EnergyChOne[i] = 0;
		EnergyChTwo[i] = 0;
	}

	while (waitKey(30) != ' ');
	for (int i = 0; i <= int((hf.MemSize() - FRAMELENGTH) / FRAMESHIFT) + 1; i++) {

		//when devided with no remainder,it means data length equals to frameshift*shift times + framelength
		//which implies the pointer reach the end of the file
		if (hf.MemSize() == (i - 1) * FRAMESHIFT + FRAMELENGTH)
			break;

		for (int channel = 1; channel < hf.Channel() + 1; channel++) {

			//draw the zone of present frame in the whole wave picture
			if (channel == 1) {
				wav2img(imgWavChanOne, data.ChanOneStd, hf.MemSize(), 0, IMGMAXWIDTH, wavWindowHeight);
				putText(imgWavChanOne, "Channel 1", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				line(imgWavChanOne, Point2i(0, int(imgWavChanOne.rows*0.5)), Point2i(imgWavChanOne.cols, int(imgWavChanOne.rows*0.5)), white);	//»­ºáÖá
				line(imgWavChanOne, Point2i(int(imgWavChanOne.cols * i *FRAMESHIFT / hf.MemSize()), 0), 
					Point2i(int(imgWavChanOne.cols * i *FRAMESHIFT / hf.MemSize()), imgWavChanOne.rows), red);
				line(imgWavChanOne, Point2i(int(imgWavChanOne.cols *(i * FRAMESHIFT + FRAMELENGTH) / hf.MemSize()), 0),
					Point2i(int(imgWavChanOne.cols *(i * FRAMESHIFT + FRAMELENGTH) / hf.MemSize()), imgWavChanOne.rows), red);
			}
			else if(channel ==2) {
				wav2img(imgWavChanTwo, data.ChanTwoStd, hf.MemSize(), 0, IMGMAXWIDTH, wavWindowHeight, "blue");
				putText(imgWavChanTwo, "Channel 2", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				line(imgWavChanTwo, Point2i(0, int(imgWavChanOne.rows*0.5)), Point2i(imgWavChanOne.cols, int(imgWavChanOne.rows*0.5)), white);
				line(imgWavChanTwo, Point2i(int(imgWavChanOne.cols * i *FRAMESHIFT / hf.MemSize()), 0),
					Point2i(int(imgWavChanOne.cols * i *FRAMESHIFT / hf.MemSize()), imgWavChanOne.rows), red);
				line(imgWavChanTwo, Point2i(int(imgWavChanOne.cols *(i * FRAMESHIFT + FRAMELENGTH) / hf.MemSize()), 0),
					Point2i(int(imgWavChanOne.cols *(i * FRAMESHIFT + FRAMELENGTH) / hf.MemSize()), imgWavChanOne.rows), red);
			}

			//get one frame data
			frame.getFrameData(data,channel,hf.MemSize(),i);

			//_ASSERTE(_CrtCheckMemory());

			//draw present frame
			if (channel == 1) {
				wav2img(imgFmChanOne, frame.frameData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight);
				putText(imgFmChanOne, "original frame channel 1", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
			}
			else if (channel == 2) {
				wav2img(imgFmChanTwo, frame.frameData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight, "blue");
				putText(imgFmChanTwo, "original frame channel 2", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
			}

			//calculate autocorrelation of the frame data
			frame.calACF();

			//calculate root mean square energy
			Erms = sqrt(frame.acfData[0]);

			if (channel == 1) {
				EnergyChOne[i] = frame.acfData[0];
			}
			else if (channel == 2) {
				EnergyChTwo[i] = frame.acfData[0];
			}

			//Assuming top five frame to be silent to initialize Emax,Emin,Eint
			if (i < 5) {
				Emax[channel] += k1*Erms / 5;
				Emin[channel] += k2*Erms / 5;
				Eint[channel] += k3*Erms / 5;
				//energyThreshold[channel] += k1 * frame.acfData[0] / 5;
				isVoiced = false;
			}

			if (Erms > Emax[channel]) {
				Emax[channel] = Erms;
			}

			if (Erms < Emin[channel]) {
				if (Erms < Eint[channel])
					Emin[channel] = Eint[channel];
				else
					Emin[channel] = Erms;
				km = 1.001;
			}

			else if (Erms >= Emin[channel]) {
				km *= 1.001;
				Emin[channel] *= km;
			}

			//calculate energy threshold
			lambda = (Emax[channel] - Emin[channel]) / Emax[channel];
			energyThreshold[channel] = (1 - lambda)*Emax[channel] + lambda*Emin[channel];

			if (channel == 1) {
				cout << "km:" << km << endl;
				cout << "Emin:" << Emin[channel] << endl;
				cout << "Emax:" << Emax[channel] << endl;
				cout << "Erms:" << Erms << endl;
				cout << "Energy threshold:" << energyThreshold[channel] << endl;
			}

			if (i > 5) {
				isVoiced = false;
				if (Erms > energyThreshold[channel]) {
					if (5 <=frame.calZCR() <= ZCRThreshold) {
						//cout << "ZCR:" << frame.calZCR() << endl;
						isVoiced = true;
					}
				}
			}

			if (isVoiced == true) {

				//900HZ low-pass filtering
				for (int j = 0; j < FRAMELENGTH; j++)
					frame.frameData[j] = filter.do_sample(frame.frameData[j]);

				//Mat imgLpfChanOne, imgLpfChanTwo;
				if (channel == 1) {
					wav2img(imgLpfChanOne, frame.frameData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight);
					putText(imgLpfChanOne, "LPF channel 1", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				}
				else if (channel == 2) {
					wav2img(imgLpfChanTwo, frame.frameData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight, "blue");
					putText(imgLpfChanTwo, "LPF channel 2", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				}

				//cutoff waves
				double max = 0;
				for (int j = 0; j < FRAMELENGTH; j++)
					if (frame.frameData[j]>max)
						max = frame.frameData[j];

				double cl = 0.65*max;

				for (int j = 0; j < FRAMELENGTH; j++) {
					if (frame.frameData[j] > 0) {
						frame.frameData[j] -= cl;
						if (frame.frameData[j] < 0)
							frame.frameData[j] = 0;
					}
					else if (frame.frameData[j] < 0) {
						frame.frameData[j] += cl;
						if (frame.frameData[j] > 0)
							frame.frameData[j] = 0;
					}
				}

				if (channel == 1) {
					wav2img(imgCutOffChanOne, frame.frameData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight);
					putText(imgCutOffChanOne, "center clipping channel 1", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				}
				else if (channel == 2) {
					wav2img(imgCutOffChanTwo, frame.frameData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight, "blue");
					putText(imgCutOffChanTwo, "center clipping channel 2", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				}

				//recalculate autocorrelation
				frame.calACF();

				if (channel == 1) {
					wav2img(imgAcfChanOne, frame.acfData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight);
					putText(imgAcfChanOne, "ACF channel 1", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				}
				else if (channel == 2) {
					wav2img(imgAcfChanTwo, frame.acfData, FRAMELENGTH, 0, frameWindowWidth, frameWindowHeight, "blue");
					putText(imgAcfChanTwo, "ACF channel 2", Point2i(0, 30), FONT_HERSHEY_SIMPLEX, 0.5, white, 1);
				}

				//find the location of the next maxima
				max = -2;
				double freq = 0;
				int maxLoc = 0;

				//set start frequency as 1000 Hz
				int Start = int(hf.SamplePerSec() / 1000);

				for (int j = Start; j < FRAMELENGTH; j++)	//warning:j cannot start from 1,otherwise the maxium value would be acfData[1]
					if (frame.acfData[j]>max) {	
						max = frame.acfData[j];
						maxLoc = j;
					}

				//calculate pitch frequency
				freq = hf.SamplePerSec() / maxLoc;
				stringstream buf;
				buf << freq;
				string num = buf.str();
				if (channel == 1) {
					putText(imgWavChanOne, num, Point2i(int(imgWavChanOne.cols * i *FRAMESHIFT / hf.MemSize()), 40), FONT_HERSHEY_SIMPLEX, 0.7, red, 1);
				}
				else if (channel == 2) {
					putText(imgWavChanTwo, num, Point2i(int(imgWavChanTwo.cols * i *FRAMESHIFT / hf.MemSize()), 40), FONT_HERSHEY_SIMPLEX, 0.7, red, 1);
				}
				//cout << "ZCR:" << frame.calZCR() << endl;
			}

			else if (isVoiced == false) {
				if(channel == 1){
					imgLpfChanOne.setTo(black);
					imgCutOffChanOne.setTo(black);
					imgAcfChanOne.setTo(black);
				}
				else if (channel == 2) {
					imgLpfChanTwo.setTo(black);
					imgCutOffChanTwo.setTo(black);
					imgAcfChanTwo.setTo(black);
				}
			}

			//move all images into one image,so it is shown in one window
			if (channel == 1) {
				imgWavChanOne.copyTo(test(Rect(Point2i(0, 0), Size(imgWavChanOne.cols, imgWavChanOne.rows))));
				imgFmChanOne.copyTo(test(Rect(Point2i(0, wavWindowHeight * 2), Size(imgFmChanOne.cols, imgFmChanOne.rows))));
				imgLpfChanOne.copyTo(test(Rect(Point2i(frameWindowWidth + 15, wavWindowHeight * 2), Size(imgLpfChanOne.cols, imgLpfChanOne.rows))));
				imgCutOffChanOne.copyTo(test(Rect(Point2i((frameWindowWidth + 15) * 2, wavWindowHeight * 2), Size(imgCutOffChanOne.cols, imgCutOffChanOne.rows))));
				imgAcfChanOne.copyTo(test(Rect(Point2i((frameWindowWidth + 15) * 3, wavWindowHeight * 2), Size(imgCutOffChanOne.cols, imgCutOffChanOne.rows))));
			}
			else if (channel == 2) {
				imgWavChanTwo.copyTo(test(Rect(Rect(Point2i(0, wavWindowHeight), Size(imgWavChanTwo.cols, imgWavChanTwo.rows)))));
				imgFmChanTwo.copyTo(test(Rect(Point2i(0, wavWindowHeight * 2 + frameWindowHeight), Size(imgFmChanTwo.cols, imgFmChanTwo.rows))));
				imgLpfChanTwo.copyTo(test(Rect(Point2i(frameWindowWidth + 15, wavWindowHeight * 2 + frameWindowHeight), Size(imgLpfChanTwo.cols, imgLpfChanTwo.rows))));
				imgCutOffChanTwo.copyTo(test(Rect(Point2i((frameWindowWidth + 15) * 2, wavWindowHeight * 2 + frameWindowHeight), Size(imgCutOffChanTwo.cols, imgCutOffChanTwo.rows))));
				imgAcfChanTwo.copyTo(test(Rect(Point2i((frameWindowWidth + 15) * 3, wavWindowHeight * 2 + frameWindowHeight), Size(imgCutOffChanOne.cols, imgCutOffChanOne.rows))));
			}
	
			//_ASSERTE(_CrtCheckMemory());

			imshow("Main", test);
			waitKey(1);
			//while (waitKey(30) != ' ');
		}
	}
	
	Mat EngyChOne;
	wav2img(EngyChOne, EnergyChOne, int((hf.MemSize() - FRAMELENGTH) / FRAMESHIFT) + 1, 0, 600, 400);
	imshow("Energy Channel One", EngyChOne);

	delete[]EnergyChOne;
	delete[]EnergyChTwo;

	waitKey(0);
	//system("PAUSE");
	return 0;
}

