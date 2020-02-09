/**************************************************************************

Author:xiaozhuo12138@163.com

Description:.wav file read and write

**************************************************************************/

#include "WavIO.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "string.h"
//#include <iostream>

/// WAV audio file 'riff' section header
typedef struct
{
	char riff_char[4];
	int  package_len;
	char wave[4];
} WavRiff;

/// WAV audio file 'format' section header
typedef struct
{
	char  fmt[4];
	int   format_len;
	short format_tag;
	short channel_number;//通道数
	int   sample_rate;//采样率
	int   byte_rate;
	short byte_per_sample;
	short bits_per_sample;
} WavFormat;

/// WAV audio file 'data' section header
typedef struct
{
	char  data_field[4];
	int  data_len;
} WavData;


/// WAV audio file header
typedef struct
{
	WavRiff   riff;
	WavFormat format;
	WavData   data;
} WavHeader;

//读取wav文件的头文件
int headerread(FILE* fp, WavHeader& header);
//using namespace std;
const static char fmt[] = "fmt ";

Wave* wavefilereader_creat()
{
	Wave* pWav = NULL;
	pWav = (Wave*)malloc(sizeof(Wave));
	if (NULL == pWav) {
		return NULL;
	}
	pWav->initFlag = 0;
	pWav->bits_per_sample = 0;
	pWav->channels = 0;
	pWav->fs = 0;
	pWav->nsamples = 0;
	pWav->pData = NULL;
	return pWav;
}

int headerread(FILE* fp, WavHeader& header)
{
	fread(&(header.riff), sizeof(WavRiff), 1, fp);
	if (feof(fp)) return -1;   // unexpected eof
	char label[5];
	//string sLabel;

	// lead label string
	fread(label, 1, 4, fp);
	if (feof(fp)) return -1;   // unexpected eof
	label[4] = 0;
	while (strcmp(label, fmt) != 0)
	{
		unsigned int len, i;
		unsigned int temp;
		// unknown block

		// read length
		fread(&len, sizeof(len), 1, fp);
		if (feof(fp)) return -1;   // unexpected eof
		// scan through the block
		for (i = 0; i < len; i++)
		{
			fread(&temp, 1, 1, fp);
			if (feof(fp)) return -1;   // unexpected eof
		}
		fread(label, 1, 4, fp);
		if (feof(fp)) return -1;   // unexpected eof
		label[4] = 0;
	}
	fseek(fp, -4L, 1);
	fread(&(header.format), sizeof(WavFormat), 1, fp);
	if (feof(fp)) return -1;   // unexpected eof
	if (header.format.format_len == 16)
	{
		fread(&(header.data), sizeof(WavData), 1, fp);
		if (feof(fp)) return -1;   // unexpected eof
		while (strncmp(header.data.data_field, "data", 4) != 0)
		{
			int m = header.data.data_len / 2;
			for (int k = 0; k < m; k++)
			{
				short undata;
				fread(&undata, sizeof(short), 1, fp);
				if (feof(fp)) return -1;   // unexpected eof
			}
			fread(&(header.data), sizeof(WavData), 1, fp);
			if (feof(fp)) return -1;   // unexpected eof
		}
	}
	else if (header.format.format_len == 18)
	{
		short undata;
		fread(&undata, sizeof(short), 1, fp);
		if (feof(fp)) return -1;   // unexpected eof
		fread(&(header.data), sizeof(WavData), 1, fp);
		if (feof(fp)) return -1;   // unexpected eof
		while (strncmp(header.data.data_field, "data", 4) != 0)
		{
			int m = header.data.data_len / 2;
			for (int k = 0; k < m; k++)
			{
				fread(&undata, sizeof(short), 1, fp);
				if (feof(fp)) return -1;   // unexpected eof
			}
			fread(&(header.data), sizeof(WavData), 1, fp);
			if (feof(fp)) return -1;   // unexpected eof
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

int wavefilereader(Wave** pWav, const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (NULL == fp)
		return -1;
	int headerord = -1;
	WavHeader header;
	headerord = headerread(fp, header);
	if (headerord < 0) {
		fclose(fp);
		return -1;
	}
	//*pWav = (Wave*)malloc(sizeof(Wave));
	//if (NULL == *pWav) {
	//	fclose(fp);
	//	return -1;
	//}
	(*pWav)->bits_per_sample = header.format.bits_per_sample;
	(*pWav)->fs = header.format.sample_rate;
	(*pWav)->channels = header.format.channel_number;
	int datalen = header.data.data_len / 2;
	if (header.format.format_tag == 3)
		datalen = header.data.data_len / sizeof(float);
	(*pWav)->nsamples = datalen;
	header.data.data_len = datalen;
	(*pWav)->pData = (float*)malloc(sizeof(float)*datalen);
	if (NULL == (*pWav)->pData) {
		fclose(fp);
		return -2;
	}
	memset((*pWav)->pData, 0, sizeof(float)*datalen);
	if (header.format.format_tag == 3)
	{
		fread((*pWav)->pData, sizeof(float)*datalen, 1, fp);
		if (feof(fp)) return -1;
	}
	else
	{
		if (header.format.format_tag == 1)
		{
			short* data = (short*)malloc(sizeof(short)*datalen);
			if (NULL == data) {
				fclose(fp);
				return -2;
			}
			memset(data, 0, sizeof(short)*datalen);
			fread(data, sizeof(short)*datalen, 1, fp);
			//if (feof(fp)) return -1;
			double value = 1.0 / 32768;
			int i;
			//cout << maxdata << endl;
			for (i = 0; i < datalen; i++)
			{
				(*pWav)->pData[i] = data[i] * value;
			}

			free(data);
			data = NULL;
		}
		else
		{
			fclose(fp);
			return -2;
		}
	}
	fclose(fp);
	return 0;
}

void wavefilereader_free(Wave** pWav)
{
	if (NULL != *pWav)
	{
		if (NULL != (*pWav)->pData)
		{
			free((*pWav)->pData);
			(*pWav)->pData = NULL;
		}
	}
	free(*pWav);
	*pWav = NULL;
}
#include <math.h>
bool wavefilewrite(short* outputdata, unsigned long datalen, const char* filename, int channel, int formattag, unsigned long Fs, int bit)
{
	FILE* fp = fopen(filename, "wb");
	if (NULL == fp)
	{
		return false;
	}
	int data_perbyte = 2;
	if (formattag == 3)
	{
		data_perbyte = 4;
		bit = 32;
	}
	WavHeader header;
	strcpy(header.riff.riff_char, "RIFF");
	header.riff.package_len = datalen * data_perbyte + 44 - 8;
	strcpy(header.riff.wave, "WAVE");
	strcpy(header.format.fmt, "fmt ");
	header.format.format_len = 16;
	header.format.format_tag = formattag;
	header.format.channel_number = channel;
	header.format.sample_rate = Fs;
	header.format.byte_rate = Fs * channel * bit / 8;
	header.format.byte_per_sample = channel * bit / 8;
	header.format.bits_per_sample = bit;
	strcpy(header.data.data_field, "data");
	header.data.data_len = datalen * data_perbyte;
	fwrite(&header, sizeof(WavHeader), 1, fp);
	if (formattag == 1)
	{
		fwrite(outputdata, sizeof(short)*datalen, 1, fp);
	}
	if (formattag == 3)
	{
		float* data = new float[datalen];
		memset(data, 0, sizeof(float)*datalen);
		int maxdata = (int)(pow(2, header.format.bits_per_sample - 1));
		double coef = 1.0 / maxdata;
		for (unsigned long i = 0; i < datalen; i++)
			data[i] = (outputdata[i] * coef);
		fwrite(data, sizeof(float)*datalen, 1, fp);
		delete[] data;
	}
	fclose(fp);
	return true;
}

