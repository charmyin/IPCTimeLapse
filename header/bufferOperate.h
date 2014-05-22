struct tcpRequire{
	unsigned int firstInt;
	unsigned int secondInt;
	unsigned int thirdInt;
	unsigned int fourthInt;
	//jsonparam size, shuld shift left 20 bit. eg. 0x01000000;
	unsigned int jsonSize;

};

int getSendDataInBinary(char *buff, struct tcpRequire testStruct);
