## mmse谱减法
### 基本原理
1. mmse 谱减法的基本原理是用带噪语音谱减去估计噪声谱后得到的频谱乘以一个增益来作为语音谱的估计
2. 基本假设：噪声是与语音不相关（频谱分量统计独立）的加性噪音
3. mmse谱减法与基本谱减法差别：1：mmse谱减将减完的谱乘以增益作为语音谱的估计，增益由mmse得到，基本谱减法没有乘以增益这一步。2.谱下界不一样。
## mmse估计器和map估计器
### 基本原理
1. 将带噪语音谱乘上一个增益来计算纯净语音谱；
2. 与谱减法的差别：没有减去噪声谱的步骤；直接乘上增益
### fdenoise 文件结构
- 代码都放在src文件中。
denoise.cpp 具体的频域降噪算法  
denoise.h   具体的频域降噪算法  
kiss_fft.c  fft计算库  
kiss_fft.h  fft计算库  
 kiss_fft_guts.h fft计算库  
spectraldenoise.cpp 抽象类 spectraldenoise  
spectraldenosie.h  抽象类 spectraldenoise  
WavIO.cpp  
WavIO.h  
- test 是一个音频案例  
### fdenoise 类结构
spectraldenoise 虚基类，封装了频域降噪方法的基本步骤；  
ss： mmse-谱减法类；  
logmmse： logmmse估计器降噪法；  
map：map估计器降噪法  
### fdenoise 代码内容
- class spectraldenoise 类是一个虚基类封装了频域上降噪的基本步骤。包含ola过程，噪声谱估计和更新，音频文件的读写，窗函数。
1. 噪声谱估计函数CalNoiseSpectral：
```C++
for (int k = 0; k < 4; k++) {
			for (int i = 0; i < len; i++) { in[i].r = data[k*len + i] * win[i]; in[i].i = 0; }
			kiss_fft(cfg, in, out);
			for (int i = 0; i < len; i++) {
				NoiseSpectral[i] += sqrt((out[i].r*out[i].r + out[i].i*out[i].i));
			}
		}
		for (int i = 0; i < len; i++) {
			NoiseSpectral[i] = NoiseSpectral[i] * 0.25;
			NoisePower += NoiseSpectral[i];
		}
	}
```
取语音的前四帧作为噪声谱的估计。
```C++
if ((Power / NoisePower) < threshold) {
			NoisePower = 0;
			for (int i = 0; i < len; i++) {
				NoiseSpectral[i] = (0.1* spectral[i] + 0.9*NoiseSpectral[i]);
				NoisePower += NoiseSpectral[i];
			}
		}
```
基于能量的噪声更新，并进行平滑处理，平滑系数为0.9

2. ola（overlap-add）
```C++
for (long long k = 0; k < length / len1; k++) {
		memmove(indata, indata + len1, len2 * sizeof(float));
		memmove(indata + len2, data + k * len1, len1 * sizeof(float));
    .....
  }
```
每次读取半帧半帧。
```C++
for (int i = 0; i < len1; i++) {
			output[k*len1 + i] = out[i].r + old_out[i].r;
		}
		memmove(old_out, out + len1, len1 * sizeof(kiss_fft_cpx));
```
每次更新半帧，并前一帧的后半帧相加。

3. CalSpeechSpectral(float* Speech,const float*spectral)进行语音谱计算，是一个纯虚函数，需要实现频域上的降噪算法的时候，继承并实现功能就可以了。
