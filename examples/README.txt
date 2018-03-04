-- instructions --

0. install the dependencies and compile this project

- install ffmepg from source (I used version 3.4.x)
- install mplayer (for debugging)
- compile this project!

1. dump the example .mp4 file to a raw.

$ ./dump2bgra.sh
$ ./play_raw.sh video_5sec.bgra

2. degrade the video

$ ../src/hw/degrade_raw_video 48 video_5sec.bgra degraded.bgra
$ ./play_raw.sh degraded.bgra
