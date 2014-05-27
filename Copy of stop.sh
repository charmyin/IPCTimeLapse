#!/bin/bash
#############Stop the running capture program progress###########
##----Parameter:ipid eg: ./stop.sh 15

#Load config parameters
source /etc/ipcs.config

#i=0
while read line; do
#echo $line
  if [[ -n "$line" ]]; then
   sudo kill ${line}
   # ((i++))
  fi
done < ${pipeProgramPath}pids/$1.pids

#j=0
while read line1; do
#echo $line1
  if [[ -n "$line1" ]]; then
   sudo kill ${line1}
    #((j++))
  fi
done < ${pipeProgramPath}pids/${1}video.pids

#remove the pids file
sudo rm ${pipeProgramPath}pids/${1}.pids
sudo rm ${pipeProgramPath}pids/${1}video.pids
echo "$1 has been stoped!"
