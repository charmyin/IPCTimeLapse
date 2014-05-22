#!/bin/sh
##-------------Load ipcs.config file------------##
#---Explain---#
#---------IpCamera name; IpCamera Address;FirstStartDate-FirstEndDate-SecondsCostCaptureOneImage,SecondStartDate-SecondEndDate-SecondsCostCaptureOneImage;----------#
#--------cam15;192.168.109.15;20140405-20140425-30, 20140426-20140610-40, 20140611-20140701-20;-----------#
IpcsConfigFile=ipcs.config
i=0
while read line; do
	i++
	configLine[i]=$line
#  if [[ "$line" =~ ^[^#]*= ]]; then
#    name[i]=${line%% =*}
#    value[i]=${line#*= }
    #((i++))
#  fi
done < $IpcsConfigFile

#for((j=1;j<10000000;j++));do
#	echo $(configLine[j])
#done



######################Open service and capture image##########################

#sudo ./main15 &
#echo $!
#echo "--------------------"
#sudo ffmpeg -f h264 -i /tmp/ipcfifo15 -qscale:v 1 -vf fps=fps=1/20 rgb%08d.jpg &
#echo $!
#echo "++++++++++++++++++++"