#!/bin/bash

sleep 10

echo "Starting Stereo Vision System..."

/home/nico/RPI5-Backend/ToF_sensor/tof_streamer /dev/i2c-1 5005 &

/home/nico/RPI5-Backend/ToF_sensor/tof_streamer /dev/i2c-0 5006 &

(while true; do
    echo "Camera 0: waiting for client on port 8888..."
    rpicam-vid --camera 0 -t 0 --codec mjpeg --width 2592 --height 1944 --framerate 15 --listen -o tcp://0.0.0.0:8888
    echo "Camera 0: client disconnected, restarting in 1s..."
    sleep 1
done) &

(while true; do
    echo "Camera 1: waiting for client on port 8889..."
    rpicam-vid --camera 1 -t 0 --codec mjpeg --width 2592 --height 1944 --framerate 15 --listen -o tcp://0.0.0.0:8889
    echo "Camera 1: client disconnected, restarting in 1s..."
    sleep 1
done) &

wait