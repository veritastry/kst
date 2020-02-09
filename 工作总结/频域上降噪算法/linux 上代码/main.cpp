#include"spectraldenosie.h"
#include <iostream>
#include<ctime>
#include <sstream>  
#include <fstream>  
#include <string>  
#include"denoise.h"
#include <unistd.h>
#include <dirent.h>
#include<vector>
#include <sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/types.h>  
using namespace std;
void mkdirs(const char *muldir) 
{
    int i,len;
    char str[512];    
    strncpy(str, muldir, 512);
    len=strlen(str);
    for( i=0; i<len; i++ )
    {
        if( str[i]=='/' )
        {
            str[i] = '\0';
            if( access(str,0)!=0 )
            {
                mkdir( str, 0777 );
            }
            str[i]='/';
        }
    }
    if( len>0 && access(str,0)!=0 )
    {
        mkdir( str, 0777 );
    }
    return;
}
vector<string> getFiles(char* cate_dir)
{
	vector<string> files;//存放文件名
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	if((dir=opendir(cate_dir)) == NULL)
		{
		perror("Open dir error...");
			exit(1);
		}
	while ((ptr=readdir(dir)) != NULL){
		string str;
		str=ptr->d_name;
		struct stat buf;
		lstat(ptr->d_name,&buf);
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
			{
				continue;}
		if(str.find(".wav")!=string::npos){
				string str1=cate_dir;
				string str3="denoise_"+string(str1, 2,*(str1.end()-1));
				mkdirs(str3.data());
				string str2=ptr->d_name;
				str2=str1+'/'+str2;
				files.push_back(str2);
			}
		else if(S_ISDIR(buf.st_mode))
		{	std::vector<string> src;
			memset(base,'\0',sizeof(base));
			strcpy(base,cate_dir);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			src=(getFiles(base));
			files.insert(files.end(), src.begin(), src.end());
		}
	}
	closedir(dir);
	return files;
}
int main(int argc, char **argv)
{
	if (argc != 2) {
	fprintf(stderr, "usage: %s <inputpath>\n", argv[0]);
	return 1;
	}
	const char *inputpath = argv[1];
	DIR *dir;
	char basePath[1000];
	memset(basePath,'\0',sizeof(basePath));
	strcpy(basePath,inputpath);
	string outputpath;
	vector<string> files=getFiles(basePath);
	clock_t startTime, endTime;
	double time = 0;
	startTime = clock();
	for (int i=0; i<files.size(); i++){
		try {
			outputpath="denoise_"+string(files[i], 2,*(files[i].end()-1));
			ss sss(files[i].data());
			sss.denoise();
			endTime = clock();//计时结束
			time += (double)(endTime - startTime) / CLOCKS_PER_SEC;
			sss.writerfile(outputpath.data());}
		catch (...) {
			cout << outputpath << " has problem" << endl;
			continue;
		}
	}
	cout << "The run time is: " << time << "s" << endl;
	return 1;
}
