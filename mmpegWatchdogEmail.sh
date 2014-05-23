#!/bin/bash
##Author : Charmyin
####----Watch mmpeg is stoped, if it is stopped longer than 300 seconds, then restart it-----###

#Mmpeg will restarted after 300 seconds if the directory not updated
timeStopedRestart=300

#Load config file , dirpath
source /etc/ipcs.config

#Main path to save the images
#imageSaveMainDir=/home/media/dkapm1/

#Main app path
#pipeProgramPath=/home/cubie/Development/videocapture/VideoStreamCapture/

echo "Ipcamera watch is on the hole" >> /home/cubie/watchdogEmail.log
i=0
while read line; do
#echo $line
  if [[ -n "$line" ]]; then
    ipcConfigLineArray[i]=${line}
    ((i++))
  fi
done < /etc/ipcs.info

for ipcConfigLine in "${ipcConfigLineArray[@]}"
do
	#Get config file info
	#ipcConfigArray=($(echo $ipcConfigLine | tr ";" "\n"))
	IFS=';' read -ra ipcConfigArray <<< "$ipcConfigLine"
	ipNum=${ipcConfigArray[0]}
	if [ ! -d $imageSaveMainDir$ipNum ] ; then
		sudo echo "Directory $imageSaveMainDir$ipNum not exist, Camera $ipNum maybe new added, please use ./runx.sh $ipNum to start it -- `date +"%Y-%m-%d %H:%M:%S "`" | tee -a ${pipeProgramPath}/log/watchdog.log
		continue
	fi
	#Show time difference between latest directory modified time and current time
	latestDirName=`ls -t $imageSaveMainDir$ipNum | head -1`
	echo "ls -t $imageSaveMainDir$ipNum | head -1"
	echo latestDirName is $latestDirName
	latestDirModiTime=`date --utc --reference=$imageSaveMainDir${ipNum}/$latestDirName +%s`
	currentTime=`date --utc +%s`
	let "timeDifference=currentTime - latestDirModiTime"
	if [ "$timeDifference" -gt "$timeStopedRestart" ]; then

		#sudo ./restart.sh $ipNum
		sudo echo "${ipNum} is run out of time for 300 seconds, "$timeDifference=$currentTime - $latestDirModiTime",please check it!" | mailx -v -s "IPCamera ${ipNum} is wrong  " 976582265@qq.com
        echo "${ipNum} is run out of time for 300 seconds, "$timeDifference=$currentTime - $latestDirModiTime",please check it!"
		#sudo echo "Camera $ipNum stoped and then restarted at `date +"%Y-%m-%d %H:%M:%S "`" | tee -a ${pipeProgramPath}/log/watchdog.log
		sudo echo "Camera $ipNum is not working at `date +"%Y-%m-%d %H:%M:%S "`" | tee -a ${pipeProgramPath}/log/watchdog.log
	fi

	#echo $timeDifference
	
	#at now +1 minutes <<< "sudo ./restart.sh $ipNum"
done



