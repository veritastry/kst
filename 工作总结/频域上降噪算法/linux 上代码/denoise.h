#pragma once
#include"spectraldenosie.h"
// ss ÀàµÄ¶¨Òå
class ss : public spectraldenoise {
private:
	float*PrevSpeech = NULL;
public:
	ss(const char *fliename);
    int CalSpeechSpectral(float* Speech, const float*spectral);
	~ss();
};
// logmmseÀàµÄ¶¨Òå
class logmmse : public spectraldenoise {
private:
	float*PrevSpeech = NULL;
public:
	logmmse(const char*flienname);
	int CalSpeechSpectral(float* Speech, const float*spectral);
	double expp(double x);
	~logmmse();
};
class map :public spectraldenoise{
private:
	float *PrevSpeech = NULL;
public:
	map(const char *filenname);
	int CalSpeechSpectral(float* Speech, const float*spectral);
	~map();
};