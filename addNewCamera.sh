#!/bin/bash
#--------Author charmyin---------#

#Add new camera to capture images

#Load config file
source /etc/ipcs.config

#Main path to save the images
#imageSaveMainDir=/home/media/dkapm1/

#Main app path
#pipeProgramPath=/home/cubie/Development/videocapture/VideoStreamCapture/

if [ -z $1 ] ; then
 echo 'Please reenter command with the ipcamera last ip, eg. sudo ./addNewCamera.sh 18' 
 exit 0 
fi

#Delete old related files and directories
read -p "Are you sure delete old images, videos and files about ip camera $1?(yes/no)" isDelete
if [ "$isDelete" = "yes" ] ; then
	#Remove images
	sudo rm -r ${imageSaveMainDir}$1
	sudo rm -r ${imageSaveMainDir}video$1
	echo "Deleted ${imageSaveMainDir}$1 ! "
else
	echo "Do nothing!"
	exit 0
fi

#run the camera
sudo ${pipeProgramPath}timelapse.sh $1 
sudo ${pipeProgramPath}timeVideoLapse.sh $1 
