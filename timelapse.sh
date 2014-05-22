#!/bin/bash
#Author Charmyin
###########Auto run video capture program############
#Saved in directory like : /home/media/dkapm1/20140405-20140425-30
#Start all in ipcs.info : sudo ./timelapse.sh ; Start the specified ipc : sudo ./timelapse.sh 18  

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
	##intreval between shooting two images
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
	if [ "${#dateTimeIntervalIndex}" -lt "3" ]; then
		echo -e "Warning, The config line upside is out of time! \n-------------\n"
		echo  $ipcConfig
	else
		#Start the jobs
		#1.Create directory  /home/media/dkapm1/15/20140405-20140425-30
		echo "mkdir $imageSaveMainDir${ipcConfigArray[0]}/$dateTimeIntervalIndex"
		(sudo mkdir -p $imageSaveMainDir${ipcConfigArray[0]}/$dateTimeIntervalIndex)
		#2.start job 
		echo "sudo ${pipeProgramPath}main ${ipcConfigArray[1]} ${ipcConfigArray[0]} "
		(
			sudo ${pipeProgramPath}main ${ipcConfigArray[1]} ${ipcConfigArray[0]} >> ${pipeProgramPath}/log/main${ipcConfigArray[0]}.log & 
			echo $! > ${pipeProgramPath}/pids/${ipcConfigArray[0]}.pids
		)
		#(sudo mkdir /home/media/dkapm1/$dateTimeIntervalIndex)
		sleep 2
		countFile=($(ls -f ${imageSaveMainDir}${ipcConfigArray[0]}/$dateTimeIntervalIndex/ | wc -l))
		let "countFile=countFile-2"
		echo "sudo ffmpeg -f h264 -i /tmp/ipcfifo${ipcConfigArray[0]} -qscale:v 1 -f image2 -vf fps=fps=1/$imageShootInterval ${imageSaveMainDir}${ipcConfigArray[0]}/$dateTimeIntervalIndex/${countFile}%06d.jpg"
		(
			sudo ffmpeg -f h264 -i /tmp/ipcfifo${ipcConfigArray[0]} -qscale:v 1 -f image2 -vf fps=fps=1/$imageShootInterval ${imageSaveMainDir}${ipcConfigArray[0]}/$dateTimeIntervalIndex/${countFile}%06d.jpg &
			 #   2>> ${pipeProgramPath}/log/ffmpeg${ipcConfigArray[0]}.log 
			echo $! >> ${pipeProgramPath}/pids/${ipcConfigArray[0]}.pids
		)
	fi
	#echo $dateTimeIntervalIndex
done

#echo "total array elements: ${#ipcConfig[@]}"
#echo "ipcConfig[2]: ${ipcConfig[2]}"

