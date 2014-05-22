#!/bin/bash
#############Stop the running capture program progress###########
##----Parameter:ipid eg: ./stop.sh 15

#Load config file 
source /etc/ipcs.config

#Main app path
#pipeProgramPath=/home/cubie/Development/videocapture/VideoStreamCapture/

echo $1
i=0
while read line; do
#echo $line
  if [[ -n "$line" ]]; then
   sudo kill ${line}
    ((i++))
  fi
done < ${pipeProgramPath}pids/$1.pids
#remove the pids file
sudo rm ${pipeProgramPath}pids/$1.pids

