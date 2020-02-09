#pragma once
#include "pch.h"
#include"WavIO.h"
#include <iostream>
#include<armadillo>
using namespace std;
using namespace arma;
class Filter {
private:
	int denoise;
	int numnoise;
	int num_filter;
	const double convu(mat a,mat b);
	const mat noise(const Wave *pWav);
	const mat corr(const mat xn,const mat y);
	const mat  LMSfilter(const mat xn, const mat dn);
	const mat  NLMSfilter(const mat xn, const mat dn);
	const mat  BLMSfilter(const mat xn, const mat dn);
	const mat  FBLMSfilter(const mat xn, const mat dn);
	const mat RLSfilter(const mat xn, const mat dn);
	const mat FTRLSfilter(const mat xn, const mat dn);
	const mat SDAfilter(const mat xn, const mat dn);
          mat  wiener( mat xn,  mat dn);
		  mat corrdenoise(mat xn);
		  const mat iFTRLSfilter(const mat xn, const mat dn);
public:
	Filter(int denoise, int num_noise, int num_filter);
	mat Wavefiltering(Wave *pWav);

};