#include "SR.pb.h"
#include "kvp_exports.h"
#include<fstream>
#include <iostream>
#include <string>
#include <ctime>
//for socket
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <thread>
#include<fstream>
#include<vector>
#include <mutex>
#include <sstream>
#include<map>
#include <cstdlib>
#include<signal.h>
const int BUFFSIZE = 5;
const int QLEN=10;
map<string,map<float,string>>results;
int numthread=0;
using namespace SR;
using namespace std;
mutex m;
void pipehandle(int temp){
signal(SIGPIPE,pipehandle);
}
void writeResultHeader(ResultHeader* rh,int errorCode,int resultLength){
  rh->set_errorcode(errorCode);
  rh->set_resultlength(resultLength);
}//写结束头
void* sr(string vp_node,SpkTask ai,SpkResultList* TBNR,int i,int * ru,int jj){
          for(int t=0;t<jj;t++){
             if((i+t)>ai.audioinfo_size()) break; 
            cout<<"正在计算音频"<<i+t<<endl;;
          stringstream ss;
            ss<<(i+t);
            string path= ai.audioinfo(i+t).audiourl();
         string outfiles=ss.str()+".wav";
        outfiles=path;
//ffmpeg  -acodec pcm_alaw -f s16le -ar 8000 -ac 1 -i 1.pcm 11.wav
      //    ffmpeg -y -v \"quiet\" -ac 1 -i input.wav -acodec pcm_s16le -ar 8000 -map_channel 0.0.0 output.wav
//     string comd="ffmpeg  -acodec pcm_alaw -f s16le -ar 8000 -ac 1 -i "+path+" "+outfiles;
//		   system(comd.data());
          //  comd="ffmpeg -y -v \"quiet\" -ac 1 -i "+outfiles+" -acodec pcm_s16le -ar 8000 -map_channel"+outfiles;
            cout<<"workmode="<<ai.workmode()<<endl;
            if(ai.workmode()==SpkTask_WorkMode_REGISTER){
                cout<<"注册id"<<endl;
                ModelInfo info;
                string ark_data;
                if(KVP_Register_Speaker_ByFile_DSM(&info,outfiles.data(),vp_node.data(),ai.audioinfo(i+t).audioid().data(),0,0,false, ark_data)!=0){
                string logname="../log.txt";
                ofstream output1(logname, ios::out | ios::binary |ios::app);
                string error=ai.audioinfo(i+t).audioid()+":"+path+" "+"has error:"+" "+info.err_msg;
                m.lock();
                output1<<error<<endl;
                output1.close();
            //    numthread+=jj;
                m.unlock();
                 }
              }else{
                cout<<"识别开始"<<endl;
                TopSpeakerInfo info;
                vector<std::string> node_list;
                node_list.push_back(vp_node);
              if( KVP_Identify_Speaker_ByFile_DSM(&info,outfiles.data(), node_list,10,0,0,0)!=0){
                string logname="../log.txt";
                ofstream output1(logname, ios::out | ios::binary |ios::app);
                *ru++;
                string error=ai.audioinfo(i+t).audioid()+":"+path+" "+"has error:"+" "+info.err_msg;
                m.lock();
                output1<<error<<endl;
                output1.close();
                m.unlock();
              }
              m.lock();
              for(int ii=0;ii<10;ii++){
              SpkResult* trhr =TBNR->add_resultlist(); 
              trhr->set_audioid(ai.audioinfo(i+t).audioid());
              trhr->set_speaker(info.scores[ii].spkid);
              trhr->set_score(info.scores[ii].score);
              trhr->set_errorcode(info.err_code);
              string spkid=info.scores[ii].spkid;
              cout<<"第 "<<i+t<<"音频结果："<<"spkid="<<spkid<<endl;
              results[spkid].insert(pair<float,string>(info.scores[ii].score,ai.audioinfo(i+t).audioid()));
              }
              m.unlock();
          }
        // string comd2="rm "+outfiles;
       //  system(comd2.data());
        }
        m.lock();
        numthread+=jj;
        cout<<"识别完成："<<numthread<<endl;
        m.unlock();
}
 int main(int argc, char **argv)
 {
  if (argc != 3) {
  fprintf(stderr, "usage: %s port maxthread\n", argv[0]);
  return 1;
  }
signal(SIGPIPE,pipehandle);
 static  string vp_node="gongxinbubisai";
  char err_msg1[256];
  char err_msg;
  KVP_Init_DSM();
  bool exist;
//  KVP_Node_Exist_DSM(vp_node.data(), exist);
  int isdect=0;
  cout<<"是否新建节点，1：是；其他：否"<<endl;
  cin>>isdect;
  if(isdect==1) KVP_Node_Delete_DSM(vp_node.data(),err_msg1);
  KVP_Node_Exist_DSM(vp_node.data(), exist);
  if(exist==false)  KVP_Node_Insert_DSM(vp_node.data(),&err_msg);
  char* port= argv[1];
  int maxthread=atoi(argv[2]);
  int listenfd;
  struct sockaddr_in seraddr;
  struct sockaddr_in clinet_addr;
  socklen_t len=sizeof(sockaddr);
  int connfd ;
  bool task_=false;
  char buff1[BUFFSIZE];
  for(int i = 0 ; i < 3;++i)
    {
        if((listenfd = socket(AF_INET,SOCK_STREAM,0)) > 0)
        {
            cout<<"create socket success..."<<endl;
            break;
        }
    }
  bzero( &seraddr, sizeof(seraddr) );
    seraddr.sin_family = AF_INET ;
    seraddr.sin_port =  htons(atoi(port));
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(listenfd,(struct sockaddr *)&seraddr, sizeof(seraddr)) < 0)
    {
        cout<<"bind address with socket failed..."<<endl;
        close(listenfd);
        return -1;
}
       if(listen(listenfd,QLEN) == -1)
    {
        cout<<"listen on socket failed..."<<endl;
        close(listenfd);
        return -1;
    }
while(1){
     if( (connfd = accept(listenfd,(struct sockaddr*)&seraddr,&len)) < 0 )
    {
        cout<<"accept the request failed"<<endl;
        close(listenfd);
        return -1;
    }
     while(1){ 
        TaskHeader th;
        SpkTask ai;
        ResultHeader trh;
        SpkResultList TBNR;
        int recv1,recv2;
        char buff1[BUFFSIZE];
        if((recv1=recv(connfd,buff1,sizeof(buff1),0))!=BUFFSIZE)
        { string logname="../log.txt";
          ofstream output1(logname, ios::out | ios::binary |ios::app);
          stringstream ss;
          ss<<connfd;
          string st="socket:"+ss.str()+":"+"任务头接收有问题";
          output1<<st<<endl;
          output1.close();
          break;
          }
        cout<<"接手头： "<<recv1<<endl;
        th.ParseFromArray(buff1,BUFFSIZE);
        cout<<"tasklength="<<th.tasklength()<<endl;
        int ru=1;
        char* buff2=new char[th.tasklength()];
        if((recv2=recv(connfd,buff2,th.tasklength(),0))!=th.tasklength())
        { cout<<"recv2="<<recv2<<endl;
          string logname="../log.txt";
          ofstream output1(logname, ios::out | ios::binary |ios::app);
          stringstream ss;
          ss<<connfd;
          string st="socket:"+ss.str()+" "+"任务体接收有问题"+"\n";
          m.lock();
          output1<<st;
          output1.close();
          m.unlock();
         break;
        }
       cout<<"接受人物体： "<<recv2<<endl;
         ai.ParseFromArray(buff2,th.tasklength());
         cout<<"ai.audioinfo_size()="<<ai.audioinfo_size()<<endl;
         int j=(ai.audioinfo_size()/maxthread);
         int jj=j;
        int mod=ai.audioinfo_size()-j*(maxthread-1);
        int i=0;
        for(int r=0;r<maxthread;r++){
          for(int i=0;i<mod;i++) jj=jj+1;
         thread task(sr,vp_node,ai,&TBNR,i,&ru,jj);
         task.detach();
         i+=jj;
         if(i>=ai.audioinfo_size()) break;
       }
       while(1) {
        if(numthread==ai.audioinfo_size())
        {
         
         if(ai.workmode()==SpkTask_WorkMode_REGISTER) cout<<"注册了： "<<numthread<<endl;
          char* buff;
        writeResultHeader(&trh,ru,TBNR.ByteSizeLong());
        buff=new char[trh.ByteSizeLong()];
        trh.SerializeToArray(buff,trh.ByteSizeLong());
        send(connfd,buff,trh.ByteSizeLong(),0);
        memset(buff,0,trh.ByteSizeLong());
        buff=new char[TBNR.ByteSizeLong()];
        TBNR.SerializeToArray(buff,TBNR.ByteSizeLong());
      int sendd= send(connfd,buff,TBNR.ByteSizeLong(),0);
       cout<<"发送消息："<<sendd<<"字节"<<endl;     
        delete buff;
        buff=NULL;   
               string sr;
               string szFileName = "../sever.txt";
               ofstream output(szFileName, ios::out | ios::binary |ios::trunc);
               map<string,map<float,string>>::iterator it;
               map<float,string>::reverse_iterator itt;
       for(it=results.begin(); it!=results.end(); it++){
               sr+="目标人"+it->first+":"+"\n";
               int ij=0;
               for(itt=it->second.rbegin();itt!=it->second.rend();itt++){
                    if(ij==10) break;
                      sr+=(itt)->second+";"+"\n";
                      ij++; }
           } 
        m.lock();
        output<<sr<<endl;
       numthread=0;
        m.unlock();
      break;
          }

        }     
  
        delete buff2;
        buff2=NULL;
     }
}
       return 0;
}
