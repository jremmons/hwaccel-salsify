#!/bin/bash

ffmpeg -i video_5sec.mp4 -c:v rawvideo -pix_fmt yuv420p video_5sec.yuv
