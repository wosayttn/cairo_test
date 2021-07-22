# HW Alpha blending

  This sample shows you all supported hardward alpha blending operation between Video and Overlay layer.

## Demo

| Used Layer | ARGB pixel data |
| ----- | ----- |
|***Video(/dev/fb0)***|<img src="./figures/video.png?raw=true">|
|***Overlay(/dev/fb1)***|<img src="./figures/overlay.png?raw=true">|

|Operator|Result|
| --- | --- |
|***DC_BLEND_MODE_CLEAR***|<img src="./figures/0_DC_BLEND_MODE_CLEAR.png?raw=true">|
|***DC_BLEND_MODE_SRC***|<img src="./figures/1_DC_BLEND_MODE_SOURCE.png?raw=true">|
|***DC_BLEND_MODE_DST***|<img src="./figures/2_DC_BLEND_MODE_DEST.png?raw=true">|
|***DC_BLEND_MODE_SRC_OVER***|<img src="./figures/3_DC_BLEND_MODE_SOURCE_OVER.png?raw=true">|
|***DC_BLEND_MODE_DST_OVER***|<img src="./figures/4_DC_BLEND_MODE_DEST_OVER.png?raw=true">|
|***DC_BLEND_MODE_SRC_IN***|<img src="./figures/5_DC_BLEND_MODE_SOURCE_IN.png?raw=true">|
|***DC_BLEND_MODE_DST_IN***|<img src="./figures/6_DC_BLEND_MODE_DEST_IN.png?raw=true">|
|***DC_BLEND_MODE_SRC_OUT***|<img src="./figures/7_DC_BLEND_MODE_SOURCE_OUT.png?raw=true">|


## Video Clips
[![All Alpha blending mode](https://img.youtube.com/vi/NcBkYnoKQFU/0.jpg)](https://www.youtube.com/watch?v=NcBkYnoKQFU)

### Running Script
```
#!/bin/sh

# Render on Video layer 
./vc8000-h264 -d /dev/fb0 -v /dev/video0 -i baby_d1.264 -f argb888 -x 152 -y 60 -w 720 -h 480 &

# Render on Overlay layer
./cairo_alphablending
```

### Description
The demo plays a H264 elementary stream file and render in specific rectangle view window on video surface using vc8000-h264 program.

More, this demo also put an ARGB image on overlay surface using cairo_alphablending program and switchs all supported alpha blend operators and render out result on LCD panel automatically.

Below is the H264 ES file information.

|H.264||
| --- | --- |
|***H.264 profile***|High@L3|
|***Reference frame***|4 frames.|
|***Width***|720 pixels|
|***Height***|480 pixels|

