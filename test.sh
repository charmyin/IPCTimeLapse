#!/bin/bash
#Author Charmyin
###########Auto run video capture program############
 
 
fileArray=($(ls -t))
a=1
for fileName in "${fileArray[@]}"
	do
	  newFileName=$(printf "%06d.jpg" ${a}) #04 pad to length of 4
	  echo " ${fileName} ${newFileName}"
	  let a=a+1
	done