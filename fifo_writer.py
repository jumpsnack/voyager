import os
import time

path = 'face_detect_fifo'

fifo = open(path, 'w+');

while True:
    fifo.write('d')
    time.sleep(0.8)