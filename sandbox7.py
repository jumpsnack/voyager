import io
import random
import picamera
import cv2
import numpy as np

'''start HERE'''
with picamera.PiCamera() as camera:
    camera.resolution = (1280, 720)
    stream = picamera.CircularIO(size=10)
    camera.start_recording(stream, format='h264')

    try:
        while True:
            camera.wait_recording(1)
            cv2.imshow('test', np.array(stream.getvalue()))
    finally:
            camera.stop_recording()
