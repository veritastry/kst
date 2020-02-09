#pragma once
#include <iostream>
#include"WavIO.h"
#include"kiss_fft.h"
#include<math.h>
#include<string.h>
class spectraldenoise {
protected:
	const double pi = 3.141592653589793238462643383279502884197169399375105820974944;
	int len;
	long long length;
	float* win = NULL;
	float*data = NULL;
	float*output = NULL;
	float*NoiseSpectral = NULL;
	kiss_fft_cfg cfg;
	kiss_fft_cfg icfg;
	double  NoisePower = 0;
	int fs;
	int HamWindow(float *hamWin, int frameSize);
public:
	spectraldenoise(const char* filename);
	int CalNoiseSpectral(int is_updata, const float*spectral);
	int writerfile(const char*outname);
	int denoise();
	virtual int CalSpeechSpectral(float* Speech,const float*spectral)=0;
	~spectraldenoise();

};
