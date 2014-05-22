/*
 ============================================================================
 Name        : CTest.c
 Author      : Charmyin
 Version     :
 Copyright   : NO COPYRIGHT
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "header/bufferOperate.h"

int testmain()
{
	//prepare struct data
	struct tcpRequire testStruct;
	testStruct.firstInt=0x000000ff;
	testStruct.secondInt=0x00000000;
	testStruct.thirdInt=0x0;
	testStruct.fourthInt=0x03e80000;
	char tempJson[] = "{ \"EncryptType\" : \"MD5\", \"LoginType\" : \"DVRIPWeb\", \"PassWord\" : \"6QNMIQGe\", \"UserName\" : \"admin\" }\n";
	//json string length
	int tempJsonSize=strlen(tempJson);
	//Whole struct serilized size
	unsigned char buff[tempJsonSize+20];
	testStruct.jsonSize=tempJsonSize;//offset 24 bit
	//set jsonParam

	//Set structs data to buff in binary
	getSendDataInBinary(buff, testStruct);


	int i = sizeof(buff);
	int j;
  /*  for(j=0; j<i; j++){
    	printf("%d char is : %02x\n", j+1, buff[j]);
    }*/

	//printf("buffff---%x;---\n", buff);

//	printf("sizeof char=%x;%08x;%08x;%08x;%08x;\n", testStruct.firstInt,testStruct.secondInt,testStruct.thirdInt,testStruct.fourthInt,testStruct.jsonSize);

	return 0;
}

int getSendDataInBinary(char *buff, struct tcpRequire testStruct)
{
	//printf("Size of tempJsonSize = %d\n;", sizeof(buff));
	int sizestruct = sizeof(testStruct);
	//printf("size of testStruct = %d\n;char size=%d;\n; tempJson size=%d\n;", sizeof(buff)+20, sizeof(char));

	//copy structs data to buff
	//bzero(&buff,0);
	memset(buff, 0, sizeof(buff));
	memcpy(buff, &(testStruct.firstInt), 4);
	memcpy(buff+4, &(testStruct.secondInt), 4);
	memcpy(buff+8, &testStruct.thirdInt, 4);
	memcpy(buff+12, &testStruct.fourthInt, 4);
	memcpy(buff+16, &testStruct.jsonSize, 4);

}

