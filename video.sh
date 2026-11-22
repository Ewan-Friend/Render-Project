#!/bin/bash
mkdir -p Videos
f="Videos/video.mp4"
i=1
while [ -e "$f" ]; do
    f="Videos/video$i.mp4"
    i=$((i+1))
done
ffmpeg -y -framerate 24 -i saved-frames/%06d.bmp -c:v libx264 -pix_fmt yuv420p -crf 18 "$f"
