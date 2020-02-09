#ifndef WAVIO_H_
#define WAVIO_H_


/*
  ==============================================================================
	WavIO.h
	Author:  xiaozhuo12138@163.com
  ==============================================================================
*/
/*
调用方式:
	Wave *pWav = wavefilereader_creat();
	if (wavefilereader(&pWav, "./test.wav") != 0) {
		wavefilereader_free(&pWav);
		return -1;
	}
	float *pData = pWav->pData;
	wavefilereader_free(&pWav);
*/
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	typedef struct {
		int channels;
		int bits_per_sample;
		int nsamples;
		int fs;
		float* pData;
		int initFlag;
	}Wave;
	//wav文件读取，包括头文件和wav数据
	//create 一个句柄
	Wave* wavefilereader_creat();
	//pWav:音频数据结构体
	//filename:文件全路径或相对路径
	//return 0成功
	int wavefilereader(Wave** pWav, const char* filename);
	//释放函数
	void wavefilereader_free(Wave** pWav);

	//写入wav文件
	bool wavefilewrite(short* outputdata, unsigned long datalen, const char* filename, int channel = 1, int formattag = 1, unsigned long Fs = 44100, int bit = 16);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif
