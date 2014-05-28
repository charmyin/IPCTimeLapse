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
	dateTimeNow=`date +%Y%m%d-%H%M%S`
	dateTimeIntervalArray=($(echo ${ipcConfigArray[2]} | tr "," "\n"))
	##intreval between shooting two images
	if [[ -n "${ipcConfigArray[3]}" ]]; then
		imageShootInterval=${ipcConfigArray[3]}
	else
		imageShootInterval=10
	fi
	 
	#Start the jobs
	#1.Create directory  /home/media/dkapm1/15/20140405-20140425-30
	echo "mkdir $imageSaveMainDir${ipcConfigArray[0]}/$dateTimeNow"
	#If exist? Do something to prevent this.
	
	dirCounts=`ls $imageSaveMainDir${ipcConfigArray[0]}/ | wc -l`
	 
	let dirCounts=dirCounts+1
	picDirPath=$imageSaveMainDir${ipcConfigArray[0]}/$dirCounts
	echo "$picDirPath"
	
	(sudo mkdir -p $picDirPath)
	
	#2.start job 
	echo "sudo ${pipeProgramPath}main${ipcConfigArray[0]} ${ipcConfigArray[1]} ${ipcConfigArray[0]} "
	(
		sudo ${pipeProgramPath}main${ipcConfigArray[0]} ${ipcConfigArray[1]} ${ipcConfigArray[0]} >> ${pipeProgramPath}log/main${ipcConfigArray[0]}.log & 
		#echo $! > ${pipeProgramPath}/pids/${ipcConfigArray[0]}.pids
	)
	#(sudo mkdir /home/media/dkapm1/$dateTimeIntervalIndex)
	sleep 2
	 
	echo "sudo ffmpeg${ipcConfigArray[0]} -f h264 -i /tmp/ipcfifo${ipcConfigArray[0]} -qscale:v 1 -f image2 -vf fps=fps=1/$imageShootInterval $picDirPath/%06d.jpg"
	(
		sudo ffmpeg${ipcConfigArray[0]} -f h264 -i /tmp/ipcfifo${ipcConfigArray[0]} -qscale:v 1 -f image2 -vf fps=fps=1/$imageShootInterval $picDirPath/%06d.jpg &
		#   2>> ${pipeProgramPath}/log/ffmpeg${ipcConfigArray[0]}.log 
		#echo $! >> ${pipeProgramPath}/pids/${ipcConfigArray[0]}.pids
	)
	 
	#echo $dateTimeIntervalIndex
done

#echo "total array elements: ${#ipcConfig[@]}"
#echo "ipcConfig[2]: ${ipcConfig[2]}"

