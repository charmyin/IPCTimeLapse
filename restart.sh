#!/bin/bash
##Author charmyin
#############Stop the running capture program progress###########
##----Parameter:ipid eg: ./stop.sh 15

#Load config parameters
source /etc/ipcs.config

#----------Kill the specified running progress-------------#
echo $1
i=0
while read line; do
#echo $line
  if [[ -n "$line" ]]; then
   sudo kill ${line}
    ((i++))
  fi
done < ${pipeProgramPath}/pids/$1.pids

#j=0
#while read line1; do
#echo $line1
#  if [[ -n "$line1" ]]; then
#   sudo kill ${line1}
#    #((j++))
#  fi
#done < ${pipeProgramPath}pids/${1}video.pids 

#--------------Restart the capture program with ip first id----------------#
sudo ${pipeProgramPath}timelapse.sh $1
#sudo ${pipeProgramPath}timeVideoLapse.sh $1
