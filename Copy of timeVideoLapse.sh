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
	done < ${pipeProgramPath}ipcs.info
else
	i=0
	while read line; do
	#echo $line
	  if [[ -n "$line" ]]; then
	    ipcConfigLineArray[i]=${line}
	    ((i++))
	  fi
	done < ${pipeProgramPath}ipcs.info
fi


for ipcConfigLine in "${ipcConfigLineArray[@]}"
do
	#Get config file info
	#echo -e " \n-a--------$ipcConfigLine---b-----\n "
	ipcConfigArray=($(echo $ipcConfigLine | tr ";" "\n"))
	######Find or create the save directory by the datetime of now##########
	dateTimeNow=`date +%Y%m%d`
	dateTimeIntervalArray=($(echo ${ipcConfigArray[2]} | tr "," "\n"))
	##dateTime And Interval eg. 20140405-20140425-30
	dateTimeIntervalIndex=-1
	## images shooting intreval
	imageShootInterval=10
	for dateTimeInterval in "${dateTimeIntervalArray[@]}"
	do
	  dateTimeArray=($(echo $dateTimeInterval | tr "-" "\n"))
	  #echo "$dateTimeNow ${dateTimeArray[0]} $dateTimeNow ${dateTimeArray[1]}" 
	  if [ "$dateTimeNow" -gt "${dateTimeArray[0]}" ] && [ "$dateTimeNow" -lt "${dateTimeArray[1]}" ]; then
	 	 dateTimeIntervalIndex=$dateTimeInterval
	 	 imageShootInterval=${dateTimeArray[2]}
	  fi
	  #for dateTime in "${dateTimeArray[@]}"
	  #do
	  	#echo $dateTime
	  	#if[ "$dateTimeNow" -gt ${date}]
	  #done
	  #echo $dateTimeInterval
	done
	
	#if [ "$dateTimeIntervalIndex" -eq "-1" ]; then
	#if length less than 3, it means something wrong
	if [ "${#dateTimeIntervalIndex}" -lt "3" ]; then
		echo -e "Warning, The config line upside is out of time! $ipcConfig \n-------------\n"
	else 
		#Start the jobs
		echo "sudo ${pipeProgramPath}videoCapture ${ipcConfigArray[1]} ${ipcConfigArray[0]} ${imageSaveMainDir}"
		(
			sudo ${pipeProgramPath}videoCapture ${ipcConfigArray[1]} ${ipcConfigArray[0]} ${imageSaveMainDir} >> ${pipeProgramPath}log/${ipcConfigArray[0]}video.log & 
			echo $! > ${pipeProgramPath}/pids/${ipcConfigArray[0]}video.pids
			break
		)
		#(sudo mkdir /home/media/dkapm1/$dateTimeIntervalIndex)
	fi
	#echo $dateTimeIntervalIndex
done

#echo "total array elements: ${#ipcConfig[@]}"
#echo "ipcConfig[2]: ${ipcConfig[2]}"

