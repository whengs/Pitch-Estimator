#ifndef __WAVPROCESS_H__
#define __WAVPROCESS_H__

#include<iostream>
#include<fstream>
#include<cmath>
using namespace std;

//headerfile class of .wav file
class headerFile
{
private:
	char chuckID[5] = " ";
	int fileSize;
	char typeID[5] = " ";

	char secChuchID[5] = " ";
	int wavFormSz;
	//Wave Format Info
	short int wFormTag;
	short int nChannels;
	int nSamplesPerSec;
	int nAvgBytesPerSec;
	short int nBlockAlign;
	short int wBitsPerSample;
	//short int cbSize;

	char thirdChuckID[5] = " ";
	int dataSize;
public:
	~headerFile();
	void read(fstream &f);
	void write(fstream &f);
	void show();
	int MemSize();
	int Channel();
	int BitsPerSample();
	int SamplePerSec();
};

headerFile::~headerFile() {
	delete[]chuckID;
	delete[]typeID;
	delete[]secChuchID;
}

void headerFile::read(fstream &f) {
	f.read((char *)&chuckID, 4);
	f.read((char *)&fileSize, 4);
	f.read((char *)&typeID, 4);
	f.read((char *)&secChuchID, 4);
	f.read((char *)&wavFormSz, 4);
	f.read((char *)&wFormTag, 2);
	f.read((char *)&nChannels, 2);
	f.read((char *)&nSamplesPerSec, 4);
	f.read((char *)&nAvgBytesPerSec, 4);
	f.read((char *)&nBlockAlign, 2);
	f.read((char *)&wBitsPerSample, 2);
	//f.read((char *)&cbSize, 2);	//for format WAVE_FORMAT_PCM , there is no cbsize
	f.read((char *)&thirdChuckID, 4);
	f.read((char *)&dataSize, 4);
}

void headerFile::write(fstream &f) {	//write headerfile data in binary
	f.write((char *)&chuckID, 4);
	f.write((char *)&fileSize, 4);
	f.write((char *)&typeID, 4);
	f.write((char *)&secChuchID, 4);
	f.write((char *)&wavFormSz, 4);
	f.write((char *)&wFormTag, 2);
	f.write((char *)&nChannels, 2);
	f.write((char *)&nSamplesPerSec, 4);
	f.write((char *)&nAvgBytesPerSec, 4);
	f.write((char *)&nBlockAlign, 2);
	f.write((char *)&wBitsPerSample, 2);
	//f.write((char *)&cbSize, 2);	//for format WAVE_FORMAT_PCM , there is no cbsize
	f.write((char *)&thirdChuckID, 4);
	f.write((char *)&dataSize, 4);
}

void headerFile::show() {
	cout << "chuck ID:" << chuckID << endl
		<< "file size:" << fileSize << endl
		<< "type ID:" << typeID << endl
		<< "second chuck id:" << secChuchID << endl
		<< "wave format size:" << wavFormSz << endl
		<< "wFormatTag:" << wFormTag << endl
		<< "nChannels:" << nChannels << endl
		<< "nSamplesPerSec:" << nSamplesPerSec << endl
		<< "nAvgBytesPerSec:" << nAvgBytesPerSec << endl
		<< "nBlockAlign:" << nBlockAlign << endl
		<< "wBitsPerSample:" << wBitsPerSample << endl
		//<< "cbSize£º" << cbSize << endl
		<< "third Chuck ID:" << thirdChuckID << endl
		<< "data size:" << dataSize << endl;
}
int headerFile::MemSize() {
	return dataSize / (nChannels*wBitsPerSample / 8);
}
int headerFile::Channel() {
	return nChannels;
}
int headerFile::BitsPerSample() {
	return wBitsPerSample;
}
int headerFile::SamplePerSec() {
	return nSamplesPerSec;
}

//data class in .wav file
class wavdata {
private:
	char * ChanOne8b;
	char * ChanTwo8b;
	short int * ChanOne16b;
	short int * ChanTwo16b;
	double ChanOneStdMax;
	double ChanTwoStdMax;
public:
	double *ChanOneStd;
	double *ChanTwoStd;
	wavdata(const int &Channel, const int &BitsPerSample, const int &MemSize);
	~wavdata();	//Îö¹¹º¯Êý
	int read(fstream &f, const int &Channel, const int &BitsPerSample, const int &MemSize);
	int write(fstream &f, const int &Channel, const int &BitsPerSample, const int &MemSize);
	int writeTxt(fstream &f, const int &Channel, const int &BitsPerSample, const int &MemSize);
	void standardize(const int &Channel, const int &BitsPerSample, const int &MemSize);
	void recover(const int &Channel, const int &BitsPerSample, const int &MemSize);
	//void getStdData(double *first, double *sec);
};

wavdata::wavdata(const int &Channel, const int &BitsPerSample, const int &MemSize) {
	if (Channel == 1) {

		ChanOneStd = new double[MemSize];

		if(BitsPerSample == 8){
			ChanOne8b = new char[MemSize];
		}
		else if(BitsPerSample == 16){
			ChanOne16b = new short int[MemSize];
		}
	}
	else if (Channel == 2) {

		ChanOneStd = new double[MemSize];
		ChanTwoStd = new double[MemSize];

		if (BitsPerSample == 8) {
			ChanOne8b = new char[MemSize];
			ChanTwo8b = new char[MemSize];
		}
		else if (BitsPerSample == 16) {
			ChanOne16b = new short int[MemSize];
			ChanTwo16b = new short int[MemSize];
		}
	}
}

wavdata::~wavdata() {
	delete[]ChanOne8b;
	delete[]ChanTwo8b;
	delete[]ChanOne16b;
	delete[]ChanTwo16b;
	delete[]ChanOneStd;
	delete[]ChanTwoStd;
}

int wavdata::read(fstream &f, const int &Channel, const int &BitsPerSample, const int &MemSize) {
	f.seekg(44);
	if (Channel == 1) {
		if(BitsPerSample == 8){
			for (int i = 0; i < MemSize && !f.eof(); i++) {
				f.read((char *)&ChanOne8b[i], 1);
			}
		}
		else if(BitsPerSample == 16){
			for (int i = 0; i < MemSize && !f.eof(); i++) {
				f.read((char *)&ChanOne16b[i], 2);
			}
		}
	}
	else if (Channel == 2) {
		if(BitsPerSample == 8){
			for (int i = 0; i < MemSize && !f.eof(); i++) {
				f.read((char *)&ChanOne8b[i], 1);
				f.read((char *)&ChanTwo8b[i], 1);
			}
		}
		else if(BitsPerSample == 16){
			for (int i = 0; i < MemSize && !f.eof(); i++) {
				f.read((char *)&ChanOne16b[i], 2);
				f.read((char *)&ChanTwo16b[i], 2);
			}
		}
	}
	return 1;
}

int wavdata::write(fstream &f,const int &Channel,const int &BitsPerSample,const int &MemSize) {
	f.seekp(44);
	if(Channel == 1){
		if(BitsPerSample == 8){
			for (int i = 0; i < MemSize; i++)
				f.write((char *)&ChanOne8b[i], 1);
		}
		else if(BitsPerSample == 16){
			for (int i = 0; i < MemSize; i++)
				f.write((char *)&ChanOne16b[i], 2);
		}
	}
	else if(Channel == 2){
		if (BitsPerSample == 8) {
			for (int i = 0; i < MemSize; i++) {
				f.write((char *)&ChanOne8b[i], 1);
				f.write((char *)&ChanTwo8b[i], 1);
			}
		}
		else if (BitsPerSample == 16) {
			for (int i = 0; i < MemSize; i++) {
				f.write((char *)&ChanOne16b[i], 2);
				f.write((char *)&ChanTwo16b[i], 2);
			}
		}
	}
	return 1;
}

//write original data to .txt file
int wavdata::writeTxt(fstream &f, const int &Channel, const int &BitsPerSample, const int &MemSize) {
	if (Channel == 1) {
		if (BitsPerSample == 8) {
			f << "8 bit Mem:" << '\n';
			for (int i = 0; i < MemSize; i++) {
				f << ChanOne8b[i] << '\n';
			}
		}
		else if (BitsPerSample == 16) {
			f << "16 bit Mem:" << '\n';
			for (int i = 0; i < MemSize; i++) {
				f << ChanOne16b[i] << '\n';
			}
		}
	}
	else if (Channel == 2) {
		if(BitsPerSample == 8){
			f << "first 8 bit:" << " " << "sec 8 bit:" << endl;
			for (int i = 0; i < MemSize; i++) {
				f << ChanOne8b[i] << "        " << ChanTwo8b[i] << '\n';
			}
		}
		else if(BitsPerSample == 16){
			f << "first 16 bit:" << " " << "sec 16 bit:" << endl;
			for (int i = 0; i < MemSize; i++) {
				f << ChanOne16b[i] << "        " << ChanTwo16b[i] << '\n';
			}
		}
	}
	return 1;
}

void wavdata::standardize(const int &Channel, const int &BitsPerSample, const int &MemSize) {

	if (Channel == 1) {
		if (BitsPerSample == 8) {
			for (int i = 0; i < MemSize; i++)
				ChanOneStd[i] = ChanOne8b[i];
		}
		else if (BitsPerSample == 16) {
			for (int i = 0; i < MemSize; i++)
				ChanOneStd[i] = ChanOne16b[i];
		}
	}
	else if (Channel == 2) {
		if (BitsPerSample == 8) {
			for (int i = 0; i < MemSize; i++) {
				ChanOneStd[i] = ChanOne8b[i];
				ChanTwoStd[i] = ChanTwo8b[i];
			}
		}
		if (BitsPerSample == 16) {
			for (int i = 0; i < MemSize; i++) {
				ChanOneStd[i] = ChanOne16b[i];
				ChanTwoStd[i] = ChanTwo16b[i];
			}
		}
	}

	ChanOneStdMax = 0;

	//find maxima
	for (int i = 0; i < MemSize; i++) {
		if (abs(ChanOneStd[i])>ChanOneStdMax)
			ChanOneStdMax = abs(ChanOneStd[i]);
	}

	for (int i = 0; i < MemSize; i++) {
		ChanOneStd[i] /= ChanOneStdMax;
	}

	if (Channel == 2) {

		ChanTwoStdMax = 0;

		for (int i = 0; i < MemSize; i++)
			if (abs(ChanTwoStd[i])>ChanTwoStdMax)
				ChanTwoStdMax = abs(ChanTwoStd[i]);

		for (int i = 0; i < MemSize; i++)
			ChanTwoStd[i] /= ChanTwoStdMax;
	}
}


//stdData to int wavdata
void wavdata::recover(const int &Channel, const int &BitsPerSample, const int &MemSize) {
	if (BitsPerSample == 8) {
		for (int i = 0; i < MemSize; i++)
			ChanOne8b[i] = char(ChanOneStd[i] * ChanOneStdMax);
		if (Channel == 2)
			for (int i = 0; i < MemSize; i++)
				ChanTwo8b[i] = char(ChanTwoStd[i] * ChanTwoStdMax);
	}
	else if (BitsPerSample == 16) {
		for (int i = 0; i < MemSize; i++)
			ChanOne16b[i] = short int(ChanOneStd[i] * ChanOneStdMax);
		if (Channel == 2)
			for (int i = 0; i < MemSize; i++)
				ChanTwo16b[i] = short int(ChanTwoStd[i] * ChanTwoStdMax);
	}
}

#endif // !__WAVPROCESS_H__
#pragma once
