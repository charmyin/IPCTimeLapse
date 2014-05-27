#!/bin/bash
#Author Charmyin
###########Auto run video capture program############
#Saved in directory like : /home/media/dkapm1/20140405-20140425-30
#Start all in ipcs.info : sudo ./timeVideoLapse.sh ; Start the specified ipc : sudo ./timeVideoLapse.sh 18  

#Load config parameters
source /etc/ipcs.config

sudo mount /dev/sda1 $imageSaveMainDir

if [[ -n "$1" ]]; then
	while read line; do
	  if [[ -n "$line" ]]; then
	  ipcConfigArray0=($(echo $line | tr ";" "\n"))
	  	  if [ "$1" = "${ipcConfigArray0[0]}" ]; then
		    ipcConfigLineArray[i]=${line}
		  fi
	  fi
	done < /etc/ipcs.info
else
	i=0
	while read line; do
	#echo $line
	  if [[ -n "$line" ]]; then
	    ipcConfigLineArray[i]=${line}
	    ((i++))
	  fi
	done < /etc/ipcs.info
fi


for ipcConfigLine in "${ipcConfigLineArray[@]}"
do
	#Get config file info
	#echo -e " \n-a--------$ipcConfigLine---b-----\n "
	ipcConfigArray=($(echo $ipcConfigLine | tr ";" "\n"))
	######Find or create the save directory by the datetime of now##########
	dateTimeNow=`date +%Y%m%d`
	
	#Start the jobs
	echo "sudo ${pipeProgramPath}videoCapture${ipcConfigArray[0]} ${ipcConfigArray[1]} ${ipcConfigArray[0]} ${imageSaveMainDir}"
	(
		sudo ${pipeProgramPath}videoCapture${ipcConfigArray[0]} ${ipcConfigArray[1]} ${ipcConfigArray[0]} ${imageSaveMainDir} >> ${pipeProgramPath}log/${ipcConfigArray[0]}video.log & 
	)

  done
 
