target:
	sudo gcc -lm -g -omain main.c timeOperate.c bufferOperate.c cjson/cJSON.c
	sudo gcc -lm -g  -ovideoCapture videoCapture.c  timeOperate.c bufferOperate.c cjson/cJSON.c
	sudo gcc -osmtpServer  smtpServer.c  timeOperate.c
