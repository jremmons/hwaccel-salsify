#!/bin/bash

ffmpeg -i video_5sec.mp4 -c:v rawvideo -pix_fmt bgra video_5sec.yuv
mv video_5sec.yuv video_5sec.bgra
