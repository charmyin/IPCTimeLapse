/*
 ============================================================================
 Name        : smtpServer.c
 Author      : charmyin
 Version     :
 Copyright   : Copy by yourself~
 Description : SMTP server to capture movement info from camera,
 	 	 	   then send to main program for video capturing, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include "header/timeOperate.h"

//Get last IP Integer, eg: line=192.168.1.122 ,return 122
char* getLastIPInt(char* line) {
	//char line[] = "192.22.123.11";
	char *search = ".";
	char* token;
	// Token will point to "SEVERAL".
	token = strtok(line, search);
	// Token will point to "WORDS".
	token = strtok(NULL, search);
	token = strtok(NULL, search);
	token = strtok(NULL, search);
    return token;
}

int main(){
  int serverfd;
  int cfd;
  struct sockaddr_in sadr;
  struct sockaddr_in cadr;
  socklen_t len;
  int r;
  int portnum=11225;
  char buf[102];
  //1.socket
  serverfd= socket(AF_INET, SOCK_STREAM, 0);
  if(serverfd==-1){
    printf("socket error: %m \n");
    exit(-1);
  }

  //2.bind
  sadr.sin_family=AF_INET;
  sadr.sin_port=htons(portnum);
  printf("Port Number: %d\n", portnum);
  inet_aton("192.168.109.102", &sadr.sin_addr);
  r=bind(serverfd, (struct sockaddr*)&sadr, sizeof(sadr));
  if(r==-1){
    printf("bind error: %m\n");
    exit(-1);
  }
  printf("Bind ip success!\n");
  //3.listen
  r=listen(serverfd, 10 );
  if(r==-1){
    printf("listen error: %m\n");
    exit(-1);
  }
  printf("Listen success!\n");
  //4.accept
  while(1){
    len=sizeof(cadr);
    cfd=accept(serverfd, (struct sockaddr*)&cadr, &len);
    if(cfd==-1){
    	continue;
    }
    char* lastIPInt = getLastIPInt(inet_ntoa(cadr.sin_addr));
    printf("%s -- TCP and port %d, Camera IP:%s, port:%u\n", now(), cfd, lastIPInt, ntohs(cadr.sin_port));
    char childPidPath[30];
    snprintf(childPidPath, sizeof childPidPath, "pids/%schild.pid", lastIPInt);
    //printf("path is %s\n", childPidPath);
    close(cfd);
    //Get child pid from pids/18child.pid ,
    char childPid[10];
	FILE *p;
	if((p=fopen(childPidPath,"r"))==NULL){
		printf("\nUnable to open file pids/%schild.pid\n", lastIPInt);
		continue;
	}
	fgets(childPid,10,p);
	int childPidInt = atoi(childPid);
	//printf("the pid is %d\n", childPidInt);
	fclose(p);
    //Send kill -10 1232
	kill(childPidInt, SIGUSR1);
   }
  close(serverfd);

  return 0;

}
