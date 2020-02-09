## linux上的文件夹操作
### 头文件：  
 #include <sstream\>  
 #include <fstream\>    
 #include <string\>    
 #include <unistd.h>  
 #include <dirent.h>  
 #include<vector\>
 #include <sys/stat.h\>  
 #include<fcntl.h\>  
 #include<sys/types.h\>    
1. 分级新建文件夹
```C++
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
```
如果路径包含文件名也会新建一个同名的文件夹。
2. 分级读取文件
```C++
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
```
在很多情况下readdir就可以完成分级读取文件名的工作，但在文件系统为xfs等情况下，readdir返回的文件类型（d_type可能为空），所以考虑到兼容性还是用stat比较好，但是stat有可能把所有的路径都判断为目录（fstat则会把所有的目录都判断成文件），所以像上面那样写可能更好。
### C++分行读取txt
```C++
ifstream  fin("filename.txt", ios::in);
	char  line[1024] = { 0 };
	while(fin.getline(line, sizeof(line))){
		stringstream  word(line);
		word >> in;
```
