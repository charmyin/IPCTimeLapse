#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "header/bufferOperate.h"
#include "cjson/cJSON.h"
#include "header/timeOperate.h"
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>


//Send socket data
// headData: struts data, contains size of jsonData
// jsonData: json string data, parameters upload
// handler: socket handler
int sendSocketData(struct tcpRequire headData, char *jsonData, int sfd){
  	unsigned char buffSend[20];
  	//json string length
  	if(jsonData==0)
  		headData.jsonSize=0;
	else
		headData.jsonSize=strlen(jsonData);
  	getSendDataInBinary(buffSend, headData);
  	//send head
  	int r;
  	r=send(sfd, &buffSend, 20, 0);
  	if(r==-1){
  		return -1;
  	}
  	//send json parameters
  	if(jsonData!=0){
		r=send(sfd, jsonData, headData.jsonSize, 0);
		if(r==-1)
			return -1;
  	}
  	return 0;
}

int receiveSocketStruct(struct tcpRequire *returnStruct, int sfd){
	  int r;
	  struct tcpRequire tempStruct;
	  r=recv(sfd, &(returnStruct->firstInt) ,4, MSG_WAITALL);
	  r=recv(sfd, &(returnStruct->secondInt) ,4, MSG_WAITALL);
	  r=recv(sfd, &(returnStruct->thirdInt) ,4, MSG_WAITALL);
	  r=recv(sfd, &(returnStruct->fourthInt) ,4, MSG_WAITALL);
	  r=recv(sfd, &(returnStruct->jsonSize) ,4, MSG_WAITALL);
	  return r;
}

//Get json data after struct data
//size: size of json string data
//buff: json string data
//sfd: socket file descriptor
int receiveSocketJson(int size, unsigned char *buff, int sfd){
	return  recv(sfd, buff, size, MSG_WAITALL);
}

//If videoFlag=1, video will be recorded; if videoFlag=0, only image will be captured.
int videoFlag=0;

//192.168.109.11
char* cameraIPAddress[20];
//Path to store the video
char* cameraVideoSavePath[50];
//Camera id code(Usually use the last ipaddress-11)
char* cameraIDCode[20];
//Child Pid filepath: ./pids/11child.pid
char* cameraChildPIDFilePath[50];

int videoffd;

char* videoFileName[20];

//Once capture a movement signal, then capture the video
void videoCaptureHandler(){

	 //If previous status is not video catpuring, Change video file name, and open it;
	if(videoFlag==0){
		 char * videoFileTimeForName = nowNoSignal();
		 snprintf(videoFileName, sizeof videoFileName, "%s%s.h264", cameraVideoSavePath, videoFileTimeForName);
		 videoffd=open(videoFileName, O_RDWR|O_CREAT, 0666);
	}
	printf("%s--Starting video capturing! Write to file %s\n", now(), videoFileName);
    videoFlag=1;
    //Record 20 seconds video once signal comes, if comes in the period of recording,
    //the timer will be added another 20 seconds.
    alarm(20);
}

//Alarm signal event handler, for stopping video capture when time is up
void videoTimeupAlarm(){
	videoFlag=0;
	//TODO maybe promblem occured~
	close(videoffd);
	printf("Video capturing ended! Write to file %s\n", videoFileName);
}

int main(int argc, char *argv[])
{

  //Set user defined signal
  signal(SIGUSR1, videoCaptureHandler);
  //Set alarm signal, for stopping video capture when time is up
  signal(SIGALRM,videoTimeupAlarm);
  //Set alarm signal, for stopping video capture when time is up
   signal(SIGALRM,videoTimeupAlarm);

  if(argc!=4){
	  printf("Argument 1 is ipc's ipaddress, and argv[2] is the ipc's id code(Usually use last ip address!), argv[3] is video storage parent path(/home/media/dkpm1/)\nPlease use like this: ' sudo ./videoCapture 192.168.1.109 19 /home/media/dkpm1/ ' \n");
	  return 0;
  }

  snprintf(cameraIPAddress, sizeof cameraIPAddress, "%s", argv[1]);
  snprintf(cameraIDCode, sizeof cameraIDCode, "%s", argv[2]);
  snprintf(cameraVideoSavePath, sizeof cameraVideoSavePath, "%svideo%s/", argv[3], argv[2]);
  snprintf(cameraChildPIDFilePath, sizeof cameraChildPIDFilePath, "./pids/%schild.pid", argv[2]);
  //connect handler; vsfd is used to receive video data
  int sfd, ffd, vsfd;
  //r1 is used to receive video databuff
  int r, r1;
  int len;
  int fileSize;//File size of received;
  int recvSize=0; // Number show how much bytes has been recevied; 23Bytes in 78Bytes;
  char buf[1024];
  char filename[100];
  struct sockaddr_in dr;

  sfd=socket(AF_INET, SOCK_STREAM, 0);
  if(sfd==-1){
    printf("socket error %m\n");
    exit(-1);
  }
  printf("Socket on %s created successfully!\n", cameraIPAddress);

  dr.sin_family=AF_INET;
  dr.sin_port=htons(34568);
  dr.sin_addr.s_addr=inet_addr(cameraIPAddress);
  r=connect(sfd, (struct sockaddr*)&dr, sizeof(dr));
  if(r==-1){
    printf("connect error: %m\n");
    close(sfd);
    exit(-1);
  }
   //prepare struct data
  	struct tcpRequire testStruct;
  	testStruct.firstInt=0x000000ff;
  	testStruct.secondInt=0x00000000;
  	testStruct.thirdInt=0x0;
  	testStruct.fourthInt=0x03e80000;

  	char tempJson[] = "{ \"EncryptType\" : \"MD5\", \"LoginType\" : \"DVRIPWeb\", \"PassWord\" : \"6QNMIQGe\", \"UserName\" : \"admin\" }\n";
  	//send prepared data to ipc
	if(sendSocketData(testStruct, tempJson, sfd)==-1){
		printf("Something has wrong on Sending User name and password~");
		return 0;
	}

  //Receive and analysis the data
  struct tcpRequire returnStruct;
  receiveSocketStruct(&returnStruct, sfd);
  unsigned char buff[returnStruct.jsonSize];
  if(receiveSocketJson(returnStruct.jsonSize, buff, sfd)==-1){
	printf("Something has wrong on Receiving first data~");
	return 0;
  }

  int countBuff;
  printf("Received %d string is : %s;\n", countBuff, buff);

    // Get SessionID
	cJSON *json, *jsonSessionIDObj;
	json=cJSON_Parse(buff);
	jsonSessionIDObj = cJSON_GetObjectItem(json, "SessionID");
	//Get origin sessionID
	printf("session id is : %s\n", jsonSessionIDObj->valuestring);
	char *sessionID =malloc(sizeof(sessionID));
	strcpy(sessionID, jsonSessionIDObj->valuestring);

    int sessionIDNum;
	sscanf(sessionID, "%x", &sessionIDNum);

/************************************Send Name SystemInfo*******************************************/

    //prepare struct data to request data
	struct tcpRequire secondStruct;
	secondStruct.firstInt=0x000000ff;
	secondStruct.secondInt=sessionIDNum;
	secondStruct.thirdInt=0x0;
	secondStruct.fourthInt=0x03fc0000;

	cJSON *jsonDst, *jsonSessionIDObjDst;
  	char *tempDstJson = "{ \"Name\" : \"SystemInfo\", \"SessionID\" : \"0x00000040\" }\n";

  	//Pay the value to send json
  	jsonDst= cJSON_Parse(tempDstJson);
  	jsonSessionIDObjDst = cJSON_GetObjectItem(jsonDst, "SessionID");
  	jsonSessionIDObjDst->valuestring=malloc(sizeof(sessionID));
  	strcpy(jsonSessionIDObjDst->valuestring,sessionID);

	char *requestString=cJSON_PrintUnformatted(jsonDst);

	printf("Send Name SystemInfo: %s\n",requestString);

	//Release json obj
	cJSON_Delete(json);
	cJSON_Delete(jsonDst);

	//send prepared data to ipc
	if(sendSocketData(secondStruct, requestString, sfd)==-1){
		printf("Something has wrong on Sending Name SystemInfo~");
		return 0;
	}
	//Receive the channel info and so on
   receiveSocketStruct(&returnStruct, sfd);
   unsigned char buff2[returnStruct.jsonSize];
   if(receiveSocketJson(returnStruct.jsonSize, buff2, sfd)==-1){
	  printf("Something has wrong on Receiving first data~");
	  return 0;
   }

 printf("Received string  SystemInfo is : %s;\n", buff2);

  	/************************************Send KeepAlive*******************************************/

  	secondStruct.firstInt=0x000000ff;
	secondStruct.secondInt=sessionIDNum;
	secondStruct.thirdInt=0x0;
	secondStruct.fourthInt=0x03ee0000;

  	cJSON *jsonKeepalive, *jsonSessionIDKeepalive;

	char *tempKeepaliveJson = "{ \"Name\" : \"KeepAlive\", \"SessionID\" : \"0x00000040\" }\n";

	//Pay the value to send json
	jsonKeepalive= cJSON_Parse(tempKeepaliveJson);
	jsonSessionIDKeepalive = cJSON_GetObjectItem(jsonKeepalive, "SessionID");
	jsonSessionIDKeepalive->valuestring=malloc(sizeof(sessionID));
	strcpy(jsonSessionIDKeepalive->valuestring,sessionID);

	char *requestKeepAliveString=cJSON_PrintUnformatted(jsonKeepalive);

	printf("Send KeepAlive %s\n",requestKeepAliveString);

	//Release json obj
	cJSON_Delete(jsonKeepalive);

	//send prepared data to ipc
	if(sendSocketData(secondStruct, requestKeepAliveString, sfd)==-1){
		printf("Something has wrong on Sending KeepAlive~");
		return 0;
	}

	//Receive the channel info and so on
   receiveSocketStruct(&returnStruct, sfd);
   unsigned char buff3[returnStruct.jsonSize];
   if(receiveSocketJson(returnStruct.jsonSize, buff3, sfd)==-1){
	  printf("Something has wrong on Receiving first data~");
	  return 0;
   }

   printf("Received string KeepAlive is : %s\n", buff3);

/************************************Time setting***********************************/

  	secondStruct.firstInt=0x000000ff;
	secondStruct.secondInt=sessionIDNum;
	secondStruct.thirdInt=0x00000001;
	secondStruct.fourthInt=0x06360000;

  	cJSON *jsonSetTime, *jsonSessionIDSetTime, *jsonOPTimeSettingNoRTCSetTime;
  	char * formatRealTime = now();

	char *tempSetTimeJson = "{ \"Name\" : \"OPTimeSettingNoRTC\", \"OPTimeSettingNoRTC\" : \"20140313 08:03:07\", \"SessionID\" : \"0x89\" }\n";

	//Pay the value to send json
	jsonSetTime= cJSON_Parse(tempSetTimeJson);
	jsonSessionIDSetTime = cJSON_GetObjectItem(jsonSetTime, "SessionID");
	jsonSessionIDSetTime->valuestring = malloc(sizeof(sessionID));
	strcpy(jsonSessionIDSetTime->valuestring,sessionID);

	jsonOPTimeSettingNoRTCSetTime = cJSON_GetObjectItem(jsonSetTime, "OPTimeSettingNoRTC");
	printf("size of now ===== %d\n", strlen( formatRealTime));
	jsonOPTimeSettingNoRTCSetTime->valuestring = malloc(strlen(formatRealTime));
	strcpy(jsonOPTimeSettingNoRTCSetTime->valuestring,formatRealTime);

	char *requestSetTimeString=cJSON_PrintUnformatted(jsonSetTime);

	printf("Time setting %s\n",requestSetTimeString);

	//Release json obj
	cJSON_Delete(jsonSetTime);

	//send prepared data to ipc
	if(sendSocketData(secondStruct, requestSetTimeString, sfd)==-1){
		printf("Something has wrong on Sending Time setting~");
		return 0;
	}

	/***************************Send name & session id again*****************************/

	secondStruct.firstInt=0x000000ff;
	secondStruct.secondInt=sessionIDNum;
	secondStruct.thirdInt=0x0;
	secondStruct.fourthInt=0x05dc0000;

	//free(tempJson);
	tempDstJson = "{ \"Name\" : \"\", \"SessionID\" : \"0x00000040\" }\n";

	//Pay the value to send json
	jsonDst= cJSON_Parse(tempDstJson);
	jsonSessionIDObjDst = cJSON_GetObjectItem(jsonDst, "SessionID");
	jsonSessionIDObjDst->valuestring= malloc(sizeof(sessionID));
	strcpy(jsonSessionIDObjDst->valuestring, sessionID);

	requestString=cJSON_PrintUnformatted(jsonDst);

	printf("Send name & session id again : %s\n",requestString);

	//Release json obj
	cJSON_Delete(jsonDst);

	//send prepared data to ipc
	if(sendSocketData(secondStruct, requestString, sfd)==-1){
		printf("Something has wrong on Sending Name SystemInfo~");
		return 0;
	}

	secondStruct.firstInt=0x000000ff;
	secondStruct.secondInt=sessionIDNum;
	secondStruct.thirdInt=0x0;
	secondStruct.fourthInt=0x061a0000;
	//send prepared data to ipc
	requestString=0;
	if(sendSocketData(secondStruct, requestString, sfd)==-1){
		printf("Something has wrong on Sending Name SystemInfo~");
		return 0;
	}

	//Receive the last info
   receiveSocketStruct(&returnStruct, sfd);
   unsigned char buff4[returnStruct.jsonSize];
   if(receiveSocketJson(returnStruct.jsonSize, buff4, sfd)==-1){
	  printf("Something has wrong on Receiving first data~");
	  return 0;
   }

   printf("Received string Last 1 is : %s\n", buff4);

	//Receive the last info
  receiveSocketStruct(&returnStruct, sfd);
  unsigned char buff5[returnStruct.jsonSize];
  if(receiveSocketJson(returnStruct.jsonSize, buff5, sfd)==-1){
	  printf("Something has wrong on Receiving first data~");
	  return 0;
  }

  printf("Received string Last 2 is : %s\n", buff5);

//Create thread to receive video stream
int pidFork;
if((pidFork=fork())==0){
	//socket 2 used for receving avc stream(.h264)
      int sfd2;
	  struct sockaddr_in dr2;

	  sfd2=socket(AF_INET, SOCK_STREAM, 0);
	  if(sfd2==-1){
	    printf("socket error %m\n");
	    exit(-1);
	  }

	  dr2.sin_family=AF_INET;
	  dr2.sin_port=htons(34568);
	  dr2.sin_addr.s_addr=inet_addr(cameraIPAddress);
	  r=connect(sfd2, (struct sockaddr*)&dr2, sizeof(dr2));
	  if(r==-1){
	    printf("Second connect error: %m\n");
	    close(sfd2);
	    exit(-1);
	  }
	  printf("Second connect connected successfully!\n");
	  /************************************Send OPMonitor in child process*******************************************/
	    secondStruct.firstInt=0x000000ff;
	   	secondStruct.secondInt=sessionIDNum;
	   	secondStruct.thirdInt=0x0;
	   	secondStruct.fourthInt=0x05850000;

	   	char *OPMonitorJsonString = "{ \"Name\" : \"OPMonitor\", \"OPMonitor\" : { \"Action\" : \"Claim\", \"Parameter\" : { \"Channel\" : 0, \"CombinMode\" : \"NONE\", \"StreamType\" : \"Main\", \"TransMode\" : \"TCP\" } }, \"SessionID\" : \"0x43\" }\n";

	  	cJSON *jsonOPMonitor, *jsonSessionIDOPMonitor;

		//Pay the value to send json
	  	jsonOPMonitor= cJSON_Parse(OPMonitorJsonString);
	  	jsonSessionIDOPMonitor = cJSON_GetObjectItem(jsonOPMonitor, "SessionID");
	  	jsonSessionIDOPMonitor->valuestring=malloc(sizeof(sessionID));
		strcpy(jsonSessionIDOPMonitor->valuestring,sessionID);

		char *requestSessionIDOPMonitor=cJSON_PrintUnformatted(jsonOPMonitor);

		printf("Send OPMonitor in child process :: %s\n",requestSessionIDOPMonitor);

		//Release json obj
		cJSON_Delete(jsonOPMonitor);

	   	//send prepared data to ipc
	   	if(sendSocketData(secondStruct, requestSessionIDOPMonitor, sfd2)==-1){
	   		printf("Something has wrong on Sending child OPMonitor~");
	   		return 0;
	   	}

		printf("Child receive 1:");
	     //Receive the channel info and so on
	      receiveSocketStruct(&returnStruct, sfd2);
	      unsigned char buff6[returnStruct.jsonSize];
	      if(receiveSocketJson(returnStruct.jsonSize, buff6, sfd2)==-1){
			  printf("Something has wrong on Receiving first data child OPMonitor~");
			  return 0;
	      }

	      printf("Received string OPMonitor in child is : %s\n", buff6);
          sleep(1);

	       receiveSocketStruct(&returnStruct, sfd2);

	       unsigned char buffVideo1[returnStruct.jsonSize];
	       recv(sfd2,  buffVideo1, returnStruct.jsonSize, MSG_WAITALL);
	      //write(videoffd, buffVideo1, returnStruct.jsonSize);

	       //Create the dir to store video files
	       struct stat st = {0};
	       if (stat(cameraVideoSavePath, &st) == -1) {
	           mkdir(cameraVideoSavePath, 0777);
	       }
	       //Parent pid
	       int forkppid=getppid();

	       //If parent pid larger than 1, continue loop, else exit
		   while(forkppid>1){
				receiveSocketStruct(&returnStruct, sfd2);
				unsigned char buffVideo2[returnStruct.jsonSize];
				r=recv(sfd2, buffVideo2, returnStruct.jsonSize, MSG_WAITALL);
				//write(, buffVideo2, returnStruct.jsonSize);
				//If videoFlag opened, start to record to specified file.
				if(videoFlag){
					write(videoffd, buffVideo2, returnStruct.jsonSize);
					//printf("--on loading--%d\n", videoffd);
				}
				//Get parent pid
				forkppid=getppid();
		   }
	       fflush(stdout);
//	       close(fifoffd);
	  	   exit(0);
  }else{
	//Save child pid to a pids/15child.pid
	FILE *fp = fopen(cameraChildPIDFilePath, "w+");
	if(fp==-1){
		printf("%s file open error! \n", cameraChildPIDFilePath);
		exit(0);
	}
	printf("ffff %s\n", cameraChildPIDFilePath);
	fprintf(fp, "%d", pidFork);
	fclose(fp);

	sleep(1);
	 /************************************Send OPMonitor in parent  process*******************************************/
	secondStruct.firstInt=0x000000ff;
	secondStruct.secondInt=sessionIDNum;
	secondStruct.thirdInt=0x0;
	secondStruct.fourthInt=0x05820000;

	char *OPMonitorJsonStringMain = "{ \"Name\" : \"OPMonitor\", \"OPMonitor\" : { \"Action\" : \"Start\", \"Parameter\" : { \"Channel\" : 0, \"CombinMode\" : \"NONE\", \"StreamType\" : \"Main\", \"TransMode\" : \"TCP\" } }, \"SessionID\" : \"0x43\" }\n";

	cJSON *jsonOPMonitorMain, *jsonSessionIDOPMonitorMain;

	//Pay the value to send json
	jsonOPMonitorMain= cJSON_Parse(OPMonitorJsonStringMain);
	jsonSessionIDOPMonitorMain = cJSON_GetObjectItem(jsonOPMonitorMain, "SessionID");
	jsonSessionIDOPMonitorMain->valuestring=malloc(sizeof(sessionID));
	strcpy(jsonSessionIDOPMonitorMain->valuestring,sessionID);

	char *requestSessionIDOPMonitorMain=cJSON_PrintUnformatted(jsonOPMonitorMain);

	printf("Send OPMonitor in father  process :: %s\n",requestSessionIDOPMonitorMain);

	//Release json obj
	cJSON_Delete(jsonOPMonitorMain);
	//send prepared data to ipc
	if(sendSocketData(secondStruct, requestSessionIDOPMonitorMain, sfd)==-1){
		printf("Something has wrong on Sending father OPMonitor~");
		return 0;
	}

	printf("Parent receive 1:\n");
	//Receive the channel info and so on
	  receiveSocketStruct(&returnStruct, sfd);
	  unsigned char buff8[returnStruct.jsonSize];
	  if(returnStruct.jsonSize==0){
		  printf("Empty thing-----------\n");
	  }else  if(receiveSocketJson(returnStruct.jsonSize, buff8, sfd)==-1){
		  printf("Something has wrong on Receiving first data OPMonitor~");
		  return 0;
	  }

	  //printf("Received string OPMonitorMain parent is : %s\n", buff8);

	  while(1){
		    sleep(30);
		  	secondStruct.firstInt=0x000000ff;
			secondStruct.secondInt=sessionIDNum;
			secondStruct.thirdInt=0x0;
			secondStruct.fourthInt=0x03ee0000;

			//Pay the value to send json
			jsonKeepalive= cJSON_Parse(tempKeepaliveJson);
			jsonSessionIDKeepalive = cJSON_GetObjectItem(jsonKeepalive, "SessionID");
			jsonSessionIDKeepalive->valuestring=malloc(sizeof(sessionID));
			strcpy(jsonSessionIDKeepalive->valuestring,sessionID);

			requestKeepAliveString=cJSON_PrintUnformatted(jsonKeepalive);

			//printf("Send KeepAlive ----------- again  ----------%s\n",requestKeepAliveString);

			//Release json obj
			cJSON_Delete(jsonKeepalive);

			//send prepared data to ipc
			if(sendSocketData(secondStruct, requestKeepAliveString, sfd)==-1){
				printf("Something has wrong on Sending KeepAlive~");
				return 0;
			}

			//Receive the channel info and so on
		   receiveSocketStruct(&returnStruct, sfd);
		   unsigned char buff3[returnStruct.jsonSize];
		   if(receiveSocketJson(returnStruct.jsonSize, buff3, sfd)==-1){
			  printf("%s --- Something has wrong on Receiving first data~", now());
			  return 0;
		   }
		   // printf("Received string KeepAlive is : %s\n", buff3);
		   printf("VideoCapture is running at %s\n", now());
	  }

	  int *status;
	  wait(status);
	  printf("Child status is %d\n", *status);
  	  close(sfd);
  	  printf("Over~");
  }

  return 0;
}
