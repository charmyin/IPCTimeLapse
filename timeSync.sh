#!/bin/bash
#Load config properties
source /etc/ipcs.config

 
latestDirName=`ls -t $imageSaveMainDir$ipNum | head -1`
latestDirModiTime=`date --utc --reference=$imageSaveMainDir${ipNum}/$latestDirName +%s`
#echo $latestDirModiTime
#ipcConfigLineArray=( $(ls /mnt/toshibausb/*/  --full-time | grep drwxr |  awk '{print $0}') )
ipcConfigArray=16

dirs="$(ls $imageSaveMainDir${ipcConfigArray}/ | wc -l)"
 
let dirs=dirs+1
 
echo dirscount=$dirs
echo $imageSaveMainDir${ipcConfigArray}
#date -s $latestDirModiTime

#sudo ntpdate -u pool.ntp.org
