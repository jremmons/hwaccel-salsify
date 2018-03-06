#!/bin/bash

exec mplayer $1 -demuxer rawvideo -rawvideo h=720:w=1280:format=i420:fps=60
