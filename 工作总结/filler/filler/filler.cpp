#include "pch.h"
#include"WavIO.h"
#include <iostream>
#include<armadillo>
#include"Filter.h"
#include<string>
using namespace std;
int main()
{   Wave *pWav = wavefilereader_creat();
	if (wavefilereader(&pWav, "./daxiangshengke.wav") != 0) {
		cout << "读取文件失败" << endl;
		wavefilereader_free(&pWav);
		return -1;
	}
	string filtermane[10] = { "RLSfilter","LMSfilter", "FTRLSfilter", "BLMSfilter", "NLMSfilter", "FBLMSfilter", "SDAfilter", "RLSfilter", "RLSfilter", "RLSfilter" };
	int num_filter = 1;
	Filter lm(5, 5, num_filter);
	mat mat_output = lm.Wavefiltering(pWav);
	   short* pout = (short*)malloc(sizeof(short)*pWav->nsamples);
	   for (int k = 0; k <pWav->nsamples; k++) {
		   pout[k] = (32767 * mat_output(k,0));
	}
	   string outname = "./" + filtermane[num_filter- 1] + ".wav";
	   const char* p = outname.data();
   wavefilewrite(pout, pWav->nsamples, p, 1, 1, pWav->fs, pWav->bits_per_sample);
   wavefilereader_free(&pWav);
   free(pout);
   pout = NULL;
   return 0;
}


