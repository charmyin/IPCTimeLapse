#!/bin/bash
####-----------For different frames in one camera shooting circle----------------#
######------Remove all the at jobs, then reload the jobs from ipcs.info file-------######

#Load config parameters
source /etc/ipcs.config

#---------remove all the jobs
for i in `atq | awk '{print $1}'`;do atrm $i;done


#--------set all the timers
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
	ipNum=${ipcConfigArray[0]}
	#for i in "${ipcConfigArray[@]}"; do
	#    echo $i
	#done
	IFS=',' read -ra restartTimerArray <<< "${ipcConfigArray[3]}"
	#restartTimerArray=($(echo ${ipcConfigArray[3]} | tr "," "\n"))
	for restartTimer in "${restartTimerArray[@]}"
	do
		at ${restartTimer} <<< "sudo ./restart.sh $ipNum"
		echo "at ${restartTimer} <<< \"sudo ./restart.sh $ipNum\""
	done
	
	#at now +1 minutes <<< "sudo ./restart.sh $ipNum"
done




