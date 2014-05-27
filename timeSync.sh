#!/bin/bash
#Load config properties
source /etc/ipcs.config

echo ${imageSaveMainDir}
latestDirName=`ls -t $imageSaveMainDir$ipNum | head -1`
latestDirModiTime=`date --utc --reference=$imageSaveMainDir${ipNum}/$latestDirName +%s`
echo $latestDirModiTime

date -s $latestDirModiTime

#sudo ntpdate -u pool.ntp.org
