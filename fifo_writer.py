import os
import time

path = 'face_detect_fifo'

fifo = open(path, 'w+');

while True:
    fifo.write('d')
    fifo.flush()
    print "WRITE"
    time.sleep(0.8)