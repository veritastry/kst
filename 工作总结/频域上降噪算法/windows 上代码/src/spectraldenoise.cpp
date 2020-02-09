#include"spectraldenosie.h"
#include<ctime>
using namespace std;
spectraldenoise::spectraldenoise(const char* filename) {
	Wave *pWav = wavefilereader_creat();
	if (wavefilereader(&pWav, filename) != 0) {
		wavefilereader_free(&pWav);
		cout << "there is problem in this file" << endl;
		throw -1;
	}
	this->len = pWav->fs / 1000 * 25;
	if (pWav->nsamples <len*4) {
		cout << "this file is too short" << endl;
		throw -1;
	}
	if (len % 2 != 0) len = len + 1;
	this->length = pWav->nsamples;
	fs = pWav->fs;
	data = (float*)malloc(length * sizeof(float));
	output = (float*)malloc(length * sizeof(float));
	NoiseSpectral = (float*)malloc(nfft * sizeof(float));
	win = (float*)malloc(len * sizeof(float));
	if (NULL == data || NULL == output || NULL == win || NULL == NoiseSpectral) {
		cout << "cant init class" << endl;
		throw -1;
	}
	memset(NoiseSpectral, 0, nfft * sizeof(float));
	if (HamWindow(win, len) != 0) { cout << "can't init HamWindow" << endl; exit(-1); }
	cfg = kiss_fft_alloc(nfft, 0, 0, 0);
	icfg = kiss_fft_alloc(len, 1, NULL, NULL);
	memmove(data, pWav->pData, length * sizeof(float));
	wavefilereader_free(&pWav);
}
int spectraldenoise::HamWindow(float *hamWin, int frameSize) {
	if (NULL == hamWin || frameSize < 1)
		return -1;
	int i;
	float a;
	a = 2.0*this->pi / (frameSize - 1);
	for (i = 1; i <= frameSize; i++)
		hamWin[i - 1] = 0.54 - 0.46 * cos(a*(i - 1));
	return 0;
}
int spectraldenoise::CalNoiseSpectral(int is_updata,const float*spectral) {
	double threshold = 1.5;
	double Power = 0;
	kiss_fft_cpx* in = (kiss_fft_cpx *)malloc(len * sizeof(kiss_fft_cpx));
	kiss_fft_cpx* out = (kiss_fft_cpx *)malloc(nfft * sizeof(kiss_fft_cpx));
	if (NULL == in || NULL == out) return -1;
	if (!(is_updata)) {
		for (int k = 0; k < 4; k++) {
			for (int i = 0; i < len; i++) { in[i].r = data[k*len + i] * win[i]; in[i].i = 0; }
			kiss_fft(cfg, in, out);
			for (int i = 0; i < nfft; i++) {
				NoiseSpectral[i] += sqrt((out[i].r*out[i].r + out[i].i*out[i].i));
			}
		}
		for (int i = 0; i < nfft; i++) {
			NoiseSpectral[i] = NoiseSpectral[i] * 0.25;
			NoisePower += NoiseSpectral[i];
		}
	}
	else {
		for (int i = 0; i < nfft; i++) Power += spectral[i];
		if ((Power / NoisePower) < threshold) {
			NoisePower = 0;
			for (int i = 0; i < nfft; i++) {
				NoiseSpectral[i] = (0.1* spectral[i] + 0.9*NoiseSpectral[i]);
				NoisePower += NoiseSpectral[i];
			}
		}
	}
	free(in);
	free(out);
	in = out = NULL;
	return 1;
}
int spectraldenoise::denoise() {
	float *spectral = (float *)malloc(nfft * sizeof(float));
	float *Speech = (float *)malloc(nfft * sizeof(float));
	memset(spectral, 0, nfft * sizeof(float));
	double time = 0;
	double ola = 0.5;
	clock_t startTime, endTime;
	int len1 = len * ola;
	int len2 = len - len1;
	float*indata = (float *)malloc(len * sizeof(float));
	kiss_fft_cpx* in = (kiss_fft_cpx *)malloc(len * sizeof(kiss_fft_cpx));
	kiss_fft_cpx* out = (kiss_fft_cpx *)malloc(nfft * sizeof(kiss_fft_cpx));
	kiss_fft_cpx* old_out = (kiss_fft_cpx *)malloc(len1 * sizeof(kiss_fft_cpx));
	memset(old_out, 0, len1 * sizeof(kiss_fft_cpx));
	float*arg = (float *)malloc(nfft * sizeof(float));
	if (NULL == in || NULL == out || NULL == arg) return -1;
	memset(indata, 0, len * sizeof(float));
	if (CalNoiseSpectral(0, spectral) < 0) return -1;
	for (long long k = 0; k < length / len1; k++) {
		memmove(indata, indata + len1, len2 * sizeof(float));
		memmove(indata + len2, data + k * len1, len1 * sizeof(float));
		for (int i = 0; i < len; i++) {
			in[i].r = indata[i] * win[i];
			in[i].i = 0;
		}
		kiss_fft(cfg, in, out);
		for (int i = 0; i < nfft; i++) {
			spectral[i] = sqrt((out[i].r*out[i].r + out[i].i*out[i].i));
			arg[i] = atan2(out[i].i, out[i].r);
		}
		kiss_fft_cpx* iin = (kiss_fft_cpx *)malloc(nfft * sizeof(kiss_fft_cpx));
	    kiss_fft_cpx* oout = (kiss_fft_cpx *)malloc(len * sizeof(kiss_fft_cpx));
		if (CalSpeechSpectral(Speech, spectral) < 0) return -1;
		for (int i = 0; i < nfft; i++) {
			iin[i].r = ((Speech[i])) * cos(arg[i]);
			iin[i].i = ((Speech[i])) * sin(arg[i]);
		}
		kiss_fft(icfg, iin, oout);
		for (int i = 0; i < len1; i++) {
			output[k*len1 + i] = out[i].r + old_out[i].r;
		}
		memmove(old_out, out + len1, len1 * sizeof(kiss_fft_cpx));
		if (CalNoiseSpectral(1, spectral) < 0) return -1;
	}
	free(in);
	free(out);
	free(indata);
	free(old_out);
	free(arg); free(Speech);
	old_out = in = out = NULL;
	indata = Speech = arg = NULL;
}
int spectraldenoise::writerfile(const char*outname) {
	short* pout = (short*)malloc(sizeof(short)*length);
	for (int k = 0; k < length; k++) {
		pout[k] = (32767 * output[k]);
	}
	if(!wavefilewrite(pout, length, outname, 1, 1, fs)) cout<<"can't write file"<<endl;
	free(pout);
	pout = NULL;
	return 1;
}
spectraldenoise::~spectraldenoise() {
	free(cfg); free(icfg);
	free(win); free(data); free(output); free(NoiseSpectral);
	win  = data = output = NoiseSpectral = NULL;
}