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
#include <sys/types.h>
#include <sys/wait.h>

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

//192.168.109.11
char* cameraIPAddress[20];
//Fifo path(/tmp/ipcfifo) with Camera id code(Usually use the last ipaddress-11): /tmp/ipcfifo11
char* cameraFIFOPath[20];

int main(int argc, char *argv[])
{
  if(argc!=3){
	  printf("Argument 1 is ipc's ipaddress, and argv[2] is the ipc's id code(Usually use last ip address!)\nPlease use like this: sudo ./main 192.168.1.109 19 \n");
	  return 0;
  }

  snprintf(cameraIPAddress, sizeof cameraIPAddress, "%s", argv[1]);
  snprintf(cameraFIFOPath, sizeof cameraFIFOPath, "/tmp/ipcfifo%s", argv[2]);

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
if(fork()==0){
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
         int  ffd=0;
         // remove("/tmp/ipcfifo15");
          int fifocode=mkfifo(cameraFIFOPath, 0666);
          /*if(fifocode==-1){
        	  perror("mkfifo ERROR~ \n");
        	  exit(0);
          }*/
         int fifoffd=open(cameraFIFOPath,O_WRONLY);
          
         if(fifoffd==-1){
			  perror("Open fifo ERROR~ \n");
			  exit(0);
			}

	       receiveSocketStruct(&returnStruct, sfd2);

	       unsigned char buffVideo1[returnStruct.jsonSize];
	       recv(sfd2,  buffVideo1, returnStruct.jsonSize, MSG_WAITALL);
	       write(fifoffd, buffVideo1, returnStruct.jsonSize);

	       //Parent pid
	       int forkppid=getppid();
	       //If parent pid larger than 1, continue loop, else exit
		   while(forkppid>1){
				r=receiveSocketStruct(&returnStruct, sfd2);
				unsigned char buffVideo2[returnStruct.jsonSize];
				r=recv(sfd2, buffVideo2, returnStruct.jsonSize, MSG_WAITALL);
				write(fifoffd, buffVideo2, returnStruct.jsonSize);
				if(r==-1){
					printf("At %s---On receving h264 stream, there is some thing wrong\n", now());
				}
				//Get parent pid
				forkppid=getppid();
		   }

	       fflush(stdout);
	       close(fifoffd);
	       exit(0);

  }else{
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
		  sleep(20);
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
			  printf("Something has wrong on Receiving first data~");
			  return 0;
		   }
		   //printf("Received string KeepAlive is : %s\n", buff3);
		   printf("Main is running at %s\n", now());
	  }

	  int *status;
	  wait(status);
	  printf("Child status is %d\n", *status);
  	  close(sfd);
  	  printf("Over~");
  }

  return 0;
}
