#!/bin/bash

#Check whether ip camras on line

#Load config file
source /etc/ipcs.config 
 
#Main app path
#pipeProgramPath=/home/cubie/Development/videocapture/VideoStreamCapture/

i=0
while read line; do
#echo $line
  if [[ -n "$line" ]]; then
    ipcConfigLineArray[i]=${line}
    ((i++))
  fi
done < ${pipeProgramPath}ipcs.info

for ipcConfigLine in "${ipcConfigLineArray[@]}"
do
	#Get config file info
	#ipcConfigArray=($(echo $ipcConfigLine | tr ";" "\n"))
	IFS=';' read -ra ipcConfigArray <<< "$ipcConfigLine"
	ipAddress=${ipcConfigArray[1]}

	# -q quiet
	# -c5 5 times of pings to perform

	echo "Checking ${ipAddress}....."
	sudo ping -q -c3 $ipAddress > /dev/null
	 
	if [ $? -eq 0 ]; then
		echo "${ipAddress} is ok"
	else 
		echo "${ipAddress} cannot be connected!!"
	fi
	
done 
