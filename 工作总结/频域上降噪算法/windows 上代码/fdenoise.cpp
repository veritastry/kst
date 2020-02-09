#include"src/spectraldenosie.h"
#include <iostream>
#include<ctime>
#include <sstream>  
#include <fstream>  
#include <string>  
#include"src/denoise.h"
using namespace std;
int main()
{
	clock_t startTime, endTime;
	string in;
	double time = 0;
	in = "test/daxiangshengke.wav";
	string out = "test/sssss.wav";
		cout << out << endl;
		startTime = clock();
		try {
			map sss(in.data());
			sss.denoise();
			endTime = clock();//计时结束
			time += (double)(endTime - startTime) / CLOCKS_PER_SEC;
			sss.writerfile(out.data());
		}
		catch (...) {
			cout << out << "存在问题" << endl;
			return -1;
		}
	cout << "The run time is: " << time << "s" << endl;
	return 1;
}
