built with
g++ -Wall pasend.cpp -o pasend $(pkg-config --cflags --libs gstreamer-1.0)
and
g++ -Wall parecv.cpp -o parecv $(pkg-config --cflags --libs gstreamer-1.0)

running ./pasend /dev/video0 seems to be working.
the stream can be picked up with the pipeline: gst-launch-1.0 udpsrc port=5200 caps="application/x-rtp, media=video, encoding-name=H264" ! rtph264depay ! decodebin ! videoconvert ! autovideosink

however not with the parecv...
it yields the following:
going for video at: device= 5225
Now set pipeline in state playing
Running...
Error: Internal data stream error.
Returned, stopping playback
Deleting pipeline

think this is faulty caps settings..
