#!/bin/bash
#############Stop the running capture program progress###########
##----Parameter:ipid eg: ./stop.sh 15

#Kill main and ffmpeg
sudo killall -9 main$1
sudo killall -9 ffmpeg$1

#Kill video capture progress
sudo killall -9 ffmpeg$1 videoCapture$1