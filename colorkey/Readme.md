# HW Transparency using Color Key.

  This sample shows you hardward transparency operation between Video and Overlay layer.

## Demo

|Layer|ARGB pixel data|
| --- | --- |
|***Video(/dev/fb0)***|<img src="./figures/video.png?raw=true">|
|***Overlay(/dev/fb1)***|<img src="./figures/overlay.png?raw=true">|

## Video Clips
[![Overlay transparency using color-key matching](https://img.youtube.com/vi/CAVkoTZL9cQ/0.jpg)](https://www.youtube.com/watch?v=CAVkoTZL9cQ)

### Running Script
```
#!/bin/sh

# Render on Video layer 
./vc8000-h264 -d /dev/fb0 -v /dev/video0 -i baby_d1.264 -f argb888 -x 152 -y 60 -w 720 -h 480 &

# Render on Overlay layer
./cairo_colorkey
```

### Description
The demo plays a H264 elementary stream file and render in specific rectangle view window on video surface using vc8000-h264 program.

More, this demo also draw three rectangles on overlay surface, with Red, Green and Blue color respectively.
Finally, this program set red, green or blue color key setting and applies it automatically. So, you can see the overlay transparency effect in demo video clip.

Below is the H264 ES file information.
|H.264||
| --- | --- |
|***H.264 profile***|High@L3|
|***Reference frame***|4 frames.|
|***Width***|720 pixels|
|***Height***|480 pixels|
