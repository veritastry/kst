#include "pch.h"
#include"WavIO.h"
#include <iostream>
#include<armadillo>
#include"Filter.h"
using namespace std;
constexpr auto pi = 3.14;
mat hamming(int len) {
	//生成一个长度为len的hamming窗
	mat win(len, 1);
	for (int i = 0; i < len; i++) {
		win(i, 0) = 0.54 - 0.46*cos(2 * pi*i / (len - 1));
	}
	return win;
}
const mat Filter::noise(const Wave *pWav) {
	int len = (pWav->fs / 1000) * 20;
	double SNR = 3;
	mat dn(len, 1, fill::zeros);
	mat spectral;
	mat noise(len, 1, fill::zeros);
	mat yn_1(pWav->nsamples, 1, fill::zeros);
	mat yn_ola(pWav->nsamples, 1, fill::zeros);
	mat angle(len,1);
	mat ar;
	cx_mat xn(len, 1, fill::zeros);
	mat win = hamming(len);
	mat yn(pWav->nsamples, 1, fill::zeros);
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < len; i++) { dn(i, 0) = pWav->pData[j*len + i]; }
		mat dnn = dn % win;
		cx_mat FFT = fft(dnn);
		spectral = abs(FFT);
		noise += spectral;
	}
	noise = 0.25*noise;
	for (int i = 0; i < len; i++) { dn(i, 0) = pWav->pData[i]; }
	mat dn_pre = dn;
	for (int j = 4; j<int(pWav->nsamples / len); j++) {
		for (int i = 0; i < len; i++) { dn(i, 0) = pWav->pData[j*len + i]; }
		mat dnn = dn % win;
		cx_mat FFT = fft(dnn);
		spectral = abs(FFT); 
		ar= arg(FFT);
		mat power = sum(square(spectral)) / sum(square(noise));
		cout << power << endl;
		if (power(0, 0) < SNR) { noise = sqrt(0.05*spectral%spectral + 0.95*noise %noise);
		}
		xn.set_real(noise%cos(ar));
		xn.set_imag(noise%sin(ar));
		yn_1.rows(j*len, (j + 1)*len - 1)=real(ifft(xn));
}
	for (int j = 4; j<int(pWav->nsamples / len); j++) {
		for (int i = 0; i < len; i++) {int k = (j + 0.5)*len + i; dn(i, 0) = pWav->pData[k]; }
		spectral = abs(fft(dn%win));
		ar = arg(fft(dn%win));
		mat power = sum(square(spectral)) / sum(square(noise));
		if (power(0, 0) < SNR) {
			noise = sqrt(0.05*spectral%spectral + 0.95*noise %noise);
		}
		xn.set_real(noise%cos(ar));
		xn.set_imag(noise%sin(ar));
		yn_ola.rows(j*len, (j + 1)*len - 1) = real(ifft(xn));
	}
	yn.rows(0, 0.5*len - 1) = yn_1.rows(0, 0.5*len - 1);
	yn.rows(0.5*len, pWav->nsamples - 0.5*len - 1) = yn_1.rows(0.5*len, pWav->nsamples - 0.5*len - 1) + yn_ola.rows(0, pWav->nsamples - len - 1);
	yn.rows(pWav->nsamples - 0.5*len, pWav->nsamples - 1) = yn_1.rows(pWav->nsamples - 0.5*len, pWav->nsamples - 1);
	return yn;
}
const mat  Filter::LMSfilter(const mat xn, const mat dn) {
	cout << "LMS" << endl;
	mat e, w;
	int n = dn.n_rows;
	int m = 1000;
	double u;
    u=0.05;
	cout << u << endl;
	e.zeros(n, 1);
	w.zeros(m, 1);
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		mat y = w.t()*(x);
		e.row(k - 1) = dn.row(k - 1) - y;
		w = w + x * u*e.row(k - 1);
       
	}
	 cout << "w=" << w(0, 0) << endl;
	mat yn;
	yn.zeros(n, 1);
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		yn.row(k - 1) =w.t()*(x);;

	}
	
	return  dn-yn;
	
}
const mat  Filter::NLMSfilter(const mat xn, const mat dn) {
	cout << "NLMS" << endl;
	mat e, w;
	int n = dn.n_rows;
	int m = 100;
	double u = 0.1;
	e.zeros(n, 1);
	w.zeros(m, 1);
	double norm = 0;
	double alpha = 00.5;
	for (int k = m; k < n; k++) {
		norm = 0;
		mat x = xn.rows((k - m), (k - 1));
		for (int i = 0; i < 100; i++) norm += x[i, 0] * x[i, 0];
        norm +=2;
		mat y = w.t() * x;
		e.row(k - 1) = dn.row(k - 1) - y;
		w = w + x * (alpha / norm)*e.row(k - 1);
	}
	mat yn;
	yn.zeros(n, 1);
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		yn.row(k - 1) = w.t() * x;

	}
	return yn;
}
const mat Filter::RLSfilter(const mat xn, const mat dn) {
	cout << "RLS" << endl;
	mat e, w;
	int n = dn.n_rows;
	int m = 100;
	e.zeros(n, 1);//误差序列
	w.zeros(m, 1);//滤波组权值矩阵
	mat R = eye<mat>(m, m);
	for (int k = m; k < 15000; k++) {
		mat x = xn.rows(k - m, k - 1);
		mat y = w.t() * x;
		e.row(k - 1) = dn.row(k - 1) - y;
		mat rr = R * x*x.t()*R;
		mat r = x.t()*R*x;
		for (int i = 0; i < m; i++)
			for (int j = 0; j < m; j++) {
				rr(i, j) = rr(i, j) / (1 + r(0, 0));
			}
		R = R - rr;
		w = w + R * x*e.row(k - 1);
	}
	mat yn;
	yn.zeros(n, 1);
	for (int k = m; k < n; k++) {
		mat x = xn.rows(k - m, k - 1);
		yn.row(k - 1) = w.t() * x;
	}
	return dn-yn;
}
Filter::Filter(int denoise, int num_noise, int num_filter) :denoise(denoise), numnoise(numnoise), num_filter(num_filter) {}
mat Filter::Wavefiltering(Wave *pWav) {
		float *pdata = pWav->pData;
		int len = 100;
		mat xn = noise(pWav);
		mat dn(pWav->nsamples, 1);
		for (int i = 0; i < pWav->nsamples; i++) { dn(i, 0) = pdata[i];}
		mat output(pWav->nsamples,num_filter);
		switch (num_filter) {
		case 1:output.col(0) = RLSfilter(xn, dn); break;
		case 2:output.col(0) = LMSfilter(xn, dn); break;
		case 3:output.col(0) = FTRLSfilter(xn, dn); break;
		case 4:output.col(0) = BLMSfilter(xn, dn); break;
		case 5:output.col(0) = NLMSfilter(xn, dn); break;
		case 6:output.col(0) = FBLMSfilter(xn, dn); break;
		case 7:output.col(0) = SDAfilter(xn, dn); break;
		case 8:output.col(0) = wiener(xn, dn); break;
		case 9:output.col(0) = iFTRLSfilter(xn, dn); break;
		}
		return output;

	}
const mat  Filter::BLMSfilter(const mat xn, const mat dn) {
	mat e, w;
	int n = dn.n_rows;
	int m = 100;
	mat cor = corr(xn.rows(0, 100), xn.rows(0, 100));
	cx_vec eigval;
	eig_gen(eigval, cor);
	double u =1.5* 2.0/max(real(eigval));
	e.zeros(n, 1);
	w.zeros(m, 1);
	int M = 10;
	mat M_e(m, 1,fill::zeros);
	int i = 1;
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		mat y = w.t() * x;
		e.row(k - 1) = dn.row(k - 1) - y;
		M_e += x * u*e.row(k - 1);
		if (i == M) { w = w + M_e; i = 1; M_e.zeros(); }
		i++;

	}
	mat yn;
	yn.zeros(n, 1);
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		yn.row(k - 1) = w.t() * x;

	}
	return yn;
}
const mat  Filter::corr(const mat xn, const mat y) {
	//求序列的相关矩阵
	int n = xn.n_rows;
	mat r(2 * n - 1, 1);
	mat R(n, n);
	double sum = 0;
	for (int j = 0; j < n; j++) {
		for (int i = 0; i < n - j; i++)  sum += xn(i + j, 0)*y(i, 0);
		r(n - 1 - j, 0) = r(j + n - 1, 0) = sum;
		sum = 0;
	}
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			R(i, j) = r(n + i - j - 1, 0);
	
	return R;
}
const mat  Filter::FBLMSfilter(const mat xn, const mat dn) {
	   cout << "FBLMS" << endl;
	   int n = dn.n_rows;
       int M =40;
	   cx_mat w(M,1,fill::zeros);
	   double u = 0.005;
       for (int k = 0; M + k  - 1 < n; k++){
		   cx_mat x = diagmat(fft(xn.rows(k, M + k - 1)));
		   cx_mat D = fft(dn.rows(k, M + k- 1));
		   cx_mat y = x*w;
		   cx_mat E = D - y;
		   w = w + u * x.t()*E;

        }
	   cx_mat y(n,1,fill::zeros);
	   for (int k = 0; M + k  - 1 < n; k++) {
		   cx_mat x = diagmat(fft(xn.rows(k, M + k - 1)));
		    y.rows(k, M + k - 1) = ifft(x * w);
	   }
	   return real(y);
}
//const mat  Filter::NFBLMSfilter(const mat xn, const mat dn) {
//	cout << "NFBLMS" << endl;
//	int n = dn.n_rows;
//	int M = 40;
//	cx_mat w(M, 1, fill::zeros);
//	double u = 0.5;
//	mat p(M, 1, fill::zeros); 
//	mat p_1(M, 1, fill::zeros);
//	double alpha = 00.5;
//	for (int k = 0; M + k - 1 < n; k++) {
//		cx_mat x = diagmat(fft(xn.rows(k, M + k - 1)));
//		cx_mat D = fft(dn.rows(k, M + k - 1));
//		cx_mat y = x * w;
//		cx_mat E = D - y;
//		for (int i = 0; i < M; i++) {
//			p(i, 0) = (1-alpha) * p(i, 0) + alpha * (real(x(i))*real(x(i))+imag(x(i))*imag(x(i)));
//			p_1(i,0) = 1 / (p(i, 0));
//		}
//		cout << "p=" << p(0, 0) << endl;
//		cx_mat uk = u * diagmat(p_1);
//		cout << "uk=" << uk(0, 0) << endl;
//		w = w + uk * x.t()*E;
//		cout << "w=" << w(0, 0) << endl;
//	}
//	cx_mat y(n, 1, fill::zeros);
//	for (int k = 0; M + k - 1 < n; k++) {
//		cx_mat x = diagmat(fft(xn.rows(k, M + k - 1)));
//		y.rows(k, M + k - 1) = ifft(x * w);
//	}
//	return dn - real(y);
//}
const mat Filter::FTRLSfilter(const mat xn, const mat dn) {
	cout << "FTRLS" << endl;
	int m = 500;
	mat phi(m, 1, fill::zeros),wf(m, 1, fill::zeros), wb(m, 1, fill::zeros), w(m, 1, fill::zeros);//滤波组权值矩阵
	mat x(m, 1, fill::zeros),phii(m+1,1,fill::zeros);
	double efmin, ebmin, lamda = 1 ,gamma = 1;//误差序列
	efmin = ebmin= 0.0000001;
	int n = dn.n_rows;
	for (int k = 0; k < n; k++) {
		mat ef = xn(k, 0) - wf.t()*x;
		mat epsilonf = gamma * ef;
		double efminlast = efmin;
		efmin = lamda * efmin + ef(0, 0)*epsilonf(0,0);
		mat wflast = wf;
		wf = wf + phi * epsilonf;
		double u = ef(0, 0) /( lamda * efminlast);
		phii.rows(1, m) = phi - u * wflast;
		phii(0, 0) = u;
		gamma = ((lamda * efminlast) / efmin) * gamma;
		double eb = lamda * ebmin*phii(m,0);
		gamma = 1 / (1/gamma- phii(m, 0)*eb);
		double epsilonb = eb * gamma;
		ebmin = lamda * ebmin+eb* epsilonb;
		phi = phii.rows(0, m - 1)+phii(m, 0)*wb;
		wb =wb + phi*epsilonb;
		x.rows(1, m-1) = x.rows(0, m - 2);
		x(0, 0) = xn(k, 0);
		mat e = dn(k,0)- w.t()*x;
		double epsilon = e(0, 0)*gamma;
		w = w + phi*epsilon;

	}
	mat yn;
	yn.zeros(n, 1);
	x.zeros();
	for (int k = 0; k < n; k++) {
		x.rows(1, m - 1) = x.rows(0, m - 2);
		x(0, 0) = xn(k, 0);
		yn.row(k) = w.t() * x;
	}
	return yn;
}
const mat Filter::SDAfilter(const mat xn, const mat dn) {
	cout << "SDA" << endl;
	mat e, w;
	int n = dn.n_rows;
	int m = 100;
	mat cor = corr(xn.rows(0, m), xn.rows(0, m));
	double u=0.05;
	/*(1.5* 2.0 / trace(cor) < 1) ? u = 0.99999999 * 2 * 2.0 / trace(cor) : u = 0.5;*/
	cout << u << endl;
	e.zeros(n, 1);
	w.zeros(m, m);
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		mat y = dn.rows((k - m), (k - 1));
		mat p = corr(x, y);
		mat R = corr(x, x);
		w = w + (p - R * w)*u;
	}
	mat yn;
	yn.zeros(n, 1);
	for (int k = m; k < n; k++) {
		mat x = xn.rows((k - m), (k - 1));
		yn.rows((k - m), (k - 1)) = w.t()*(x);;

	}

	return yn;
}
 mat Filter::wiener( mat xn,  mat dn) {
	int n = xn.n_rows;
	int m = 100;
	mat w(m, 1);
	mat yn;
	yn.zeros(n, 1);
	for(int k=0;  k* m + m - 1< n ;k++){
	mat Rxx = corr(xn.rows(k*m, k* m +m- 1), xn.rows(k*m, k* m + m - 1));
	mat Rxd = corr(xn.rows(k*m, k* m + m - 1), dn.rows(k*m, k* m + m - 1));
	Rxd = Rxd.row(1).t();
	mat a = Rxx.i();
	w = a * Rxd;
	mat x = flipud(xn.rows(k*m, k* m + m - 1));
	yn.row(k*m) = w.t() * x;
	
	}
	return dn-yn;
}
 /*mat Filter::corrdenoise(mat xn) {
	 int n = xn.n_rows;
	 int m = 100;
	 int k = 10;
	 yn.zeros(n, 1);
	 for (int j = 0; k* m + m - 1 < n; k++) {
		 mat Rxx = corr(xn.rows(j, j + m - 1 - k), xn.rows(j + k, j + m - 1));
		 yn.row(j, j + m - 1 - k) = Rxx;

	 }
	 return yn;
 }*/
 const mat Filter::iFTRLSfilter(const mat xn, const mat dn) {
	 cout << "iFTRLS" << endl;
	 int m = 500;
	 mat phi(m, 1, fill::zeros), wf(m, 1, fill::zeros), wb(m, 1, fill::zeros), w(m, 1, fill::zeros);//滤波组权值矩阵
	 mat x(m, 1, fill::zeros), phii(m + 1, 1, fill::zeros);
	 double efmin, ebmin, lamda = 1,gamma1=1;//误差序列
	 mat gamma(2, 3, fill::ones);
	 efmin = ebmin = 0.0000001;
	 int n = dn.n_rows;
	 mat K(3, 1); 
	 K << 1.5 << 2.5 << 1;
	 mat eb3(3, 1);
	 mat epsilonb(3, 1);
	 for (int k = 0; k < n; k++) {
		 mat ef = xn(k, 0) - wf.t()*x;
		 mat epsilonf = gamma(0,2) * ef;
		 double u = ef(0, 0) / (lamda * efmin);
		 phii.rows(1, m) = phi - u * wf;
		 phii(0, 0) = u;
		 gamma(1,0) = 1 / (1/gamma(1,2)-phii(0,0)*ef(0,0));
		 efmin = 1 / (1 / (lamda*efmin) - gamma(1, 0)*phii(0, 0)*phii(0, 0));
		 wf = wf + phi * epsilonf;
		 double eb1 = lamda * ebmin*phii(m, 0);
		 mat X(m+1,1);
		 X(0, 0) = xn(k, 0);
		 X.rows(1, m) = x;
		 mat eb2 = -wb.t()*X.rows(0, m - 1) + X(m, 0);
		 
		 for (int i = 0; i < 3; i++) {
			 eb3(i, 0) = eb2(0,0) * K(0, i) + eb1 * (1 - K(0, i));
		 }
		 gamma(0, 1) = 1 / (1 / gamma(1, 0) - phii(m, 0) * eb3(2, 0));
		 for (int i = 0; i < 3; i++) {
			 epsilonb(i, 0) = eb3(i, 0) *gamma(0,1);
		 }
		 ebmin = lamda * ebmin + eb3(1, 0)*epsilonb(2, 0);
		 phi = phii.rows(0, m - 1) -phii(m, 0)*wb;
		 wb = wb + phi * epsilonb(0,0);
		 mat a = x.t()*phi;
		 gamma(0, 2) = 1/(1 + a(0,0));
		 /*x.rows(1, m - 1) = x.rows(0, m - 2);
		 x(0, 0) = xn(k, 0);*/
		 mat e = dn(k, 0) - w.t()*x;
		 double epsilon = e(0, 0)*gamma(0,2);
		 w = w + phi * epsilon;
		 x.rows(1, m - 1) = x.rows(0, m - 2);
		 x(0, 0) = xn(k, 0);
		 cout << "w=" << w(0, 0) << endl;

	 }
	 mat yn;
	 yn.zeros(n, 1);
	 x.zeros();
	 for (int k = 0; k < n; k++) {
		 x.rows(1, m - 1) = x.rows(0, m - 2);
		 x(0, 0) = xn(k, 0);
		 yn.row(k) = w.t() * x;
	 }
	 return  yn;
 }