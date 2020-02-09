#include"denoise.h"
#include<iostream>
using namespace std;
ss::ss(const char *fliename) :spectraldenoise(fliename) {
	PrevSpeech = (float *)malloc(2 * nfft * sizeof(float));
	memset(PrevSpeech, 0, 2 * nfft * sizeof(float));
}
int ss::CalSpeechSpectral(float* Speech, const float*spectral) {
	for (int w = 0; w < nfft; w++) {
		double psnr = spectral[w] * spectral[w] / NoiseSpectral[w] * NoiseSpectral[w];
		double prev_snr = PrevSpeech[w] * PrevSpeech[w] / NoiseSpectral[w] * NoiseSpectral[w];
		prev_snr = (prev_snr < log(3)) ? psnr : 30;
		double ou = (1 - 0.96)*((psnr > 1) ? (psnr - 1) : 0) + 0.96 * prev_snr;
		double temp1 = sqrt((ou*ou / (ou*ou + 0.5))*(spectral[w] * spectral[w] - NoiseSpectral[w] * NoiseSpectral[w]));
		double temp2 = 0.5*(0.2*(spectral[w]) + (PrevSpeech[w]));
		Speech[w] = (temp1 > 0.2*(spectral[w])) ? temp1 : temp2;
	}
	cout << 1 << endl;
	memmove(PrevSpeech, PrevSpeech + nfft, nfft * sizeof(float));
	memmove(PrevSpeech + nfft, Speech, nfft * sizeof(float));
	return 1;
}
ss::~ss() {
	free(PrevSpeech);
	PrevSpeech = NULL;
}
double logmmse::expp(double x)
{
	int m, i, j;
	double s, p, ep, h, aa, bb, w, xx, g, r, q;
	static double t[5] = { -0.9061798459,-0.5384693101,0.0,
						 0.5384693101,0.9061798459 };
	static double c[5] = { 0.2369268851,0.4786286705,0.5688888889,
						0.4786286705,0.2369268851 };
	m = 1;
	if (x == 0) x = 1.0e-10;
	if (x < 0.0) x = -x;
	r = 0.57721566490153286060651;
	q = r + log(x);
	h = x; s = fabs(0.0001*h);
	p = 1.0e+35; ep = 0.000001; g = 0.0;
	while ((ep >= 0.0000001) && (fabs(h) > s))
	{
		g = 0.0;
		for (i = 1; i <= m; i++)
		{
			aa = (i - 1.0)*h; bb = i * h;
			w = 0.0;
			for (j = 0; j <= 4; j++)
			{
				xx = ((bb - aa)*t[j] + (bb + aa)) / 2.0;
				w = w + (exp(-xx) - 1.0) / xx * c[j];
			}
			g = g + w;
		}
		g = g * h / 2.0;
		ep = fabs(g - p) / (1.0 + fabs(g));
		p = g; m = m + 1; h = x / m;
	}
	g = q + g;
	return(g);
}
int logmmse::CalSpeechSpectral(float* Speech, const float*spectral) {
	float aa = 0.98;
	float ksi_min = pow(10, -2.5);
	for (int w = 0; w < nfft; w++) {
		double psnr = spectral[w] * spectral[w] / NoiseSpectral[w] * NoiseSpectral[w];
		psnr = (psnr < 40)?psnr:40;
		double prev_snr = PrevSpeech[w] * PrevSpeech[w] / NoiseSpectral[w] * NoiseSpectral[w];
		double snr = aa * prev_snr + (1 - aa)*((psnr - 1 > 0) ? (psnr - 1) : 0);
		double A = snr/ (1 + snr);
		double  vk = A * psnr;
		double ei_vk = 0.5 * (-expp(vk));
		double hw = A * exp(ei_vk);
		Speech[w] = spectral[w] * hw;
	}
	memmove(PrevSpeech, PrevSpeech + nfft, nfft * sizeof(float));
	memmove(PrevSpeech + nfft, Speech, nfft * sizeof(float));
	return 1;
}
logmmse::logmmse(const char*flienname) :spectraldenoise(flienname) {
	PrevSpeech = (float *)malloc(2 * nfft * sizeof(float));
	memset(PrevSpeech, 0, 2 * nfft * sizeof(float));
}
logmmse::~logmmse() {
	free(PrevSpeech);
	PrevSpeech = NULL;
}

map::map(const char*filenname) :spectraldenoise(filenname) {
	PrevSpeech = (float *)malloc(2 * nfft * sizeof(float));
	memset(PrevSpeech, 0, 2 * nfft * sizeof(float));
}
int map::CalSpeechSpectral(float* Speech, const float*spectral) {
	float aa = 0.98;
	float ksi_min = pow(10, -2.5);
	for (int w = 0; w < nfft; w++) {
		double psnr = spectral[w] * spectral[w] / NoiseSpectral[w] * NoiseSpectral[w];
		psnr = (psnr < 40) ? psnr : 40;
		double prev_snr = PrevSpeech[w] * PrevSpeech[w] / NoiseSpectral[w] * NoiseSpectral[w];
		double snr = aa * prev_snr + (1 - aa)*((psnr - 1 > 0) ? (psnr - 1) : 0);
		double G = (snr + sqrt(snr*snr + 2 * (1 + snr)*(snr / psnr))) / (2 * (1 + snr));
		Speech[w] = G * spectral[w];
	}
	memmove(PrevSpeech, PrevSpeech + nfft, nfft * sizeof(float));
	memmove(PrevSpeech + nfft, Speech, nfft * sizeof(float));
	return 1;
}
map::~map() {

}
