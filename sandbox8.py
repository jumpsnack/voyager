from picamera.array import PiRGBArray
from picamera import PiCamera
import time
import cv2
import os
import errno
import time
import json
import threading
from multiprocessing import Queue

'//=========Configuration==========//'
conf = {
    'RES_W': 640,#resolution width
    'RES_H': 480,#resolution height
    'x': 100,#target x position
    'y': 100,#target y position
    'w': 200,#target width
    'h': 200,#target height
    'MIN_X': 0,#target x bound
    'MIN_Y': 0,#target y bound
    'metric': 2,#How much is target moved
    'prevKey': -1,#To detect whether it is continuous key input
    'timer': 0,
    'timerTh': 1
    }

conf['MAX_X'] = conf['RES_W'] - conf['w'] #target x bound
conf['MAX_Y'] = conf['RES_H'] - conf['h'] #target y bound

q_key = Queue()
'//END========Configuration========//'


'//=======FONT SET=======//'
font = cv2.FONT_HERSHEY_SIMPLEX
topLeftCornerOfText = (10,10)
fontScale = 0.3
fontColor = (0,0,255)
lineType = 1
'//END=====FONT SET=====//'


'//========CAMERA SET=========//'
camera = PiCamera()
camera.resolution = (conf['RES_W'],conf['RES_H'])
camera.framerate = 80
rawCapture = PiRGBArray(camera, size=(conf['RES_W'],conf['RES_H']))

time.sleep(0.1) #Time for preparing
'//END======CAMERA SET======//'


'//=======TARGET ACCEL.=======//'
'---------> ki yoon block make problem Never I do it, flush? ummm...'
pow_exp = 1.05
pow_base = 0

def getAcc():
    global pow_base, pow_exp

    pow_base += 1
    return int(pow_base**pow_exp)

def initBase():
    global pow_base

    pow_base = 0


'//END=====TARGET ACCEL=====//'


'//=========FIFO THREAD SET==========//'
fifo = -1
FIFO_PATH = "face_detect_fifo"
'----------> ki yoon self study cli.c call uerface.c , cli.c make it'
'//END======FIFO THREAD SET==========//'


class FifoThread(threading.Thread):
    
    def __init__(self):
        threading.Thread.__init__(self)
        self.shutdown_event = threading.Event()

    def readFifo(self):
        global fifo, q_key
        while not self.shutdown_event.is_set():
            time.sleep(0.2)
            try:
                data = os.read(fifo, 1024)
                if(len(data) != 0):
                    print('Read: "{0}"'.format(data))
                    '-------> ki yoon not understand'
                else:
                    time.sleep(1.25)
                    print('__None')
                try:
                    q_key.put(ord(data))
                    #parsed_json = json.loads(data)
                    #print('cnt: {0}, cx: {1}, cy: {2}, left: {3}, top: {4}, right: {5}, bottom: {6}'.format(
                    #    parsed_json['cnt'], parsed_json['cx'], parsed_json['cy'], parsed_json['left'], parsed_json['top'], parsed_json['right'], parsed_json['bottom']))
                except:
                    raise
                    pass
            except OSError as err:
                if err.errno == 11:
                    continue
                pass
            except:
                raise

    def openFifo(self):
        global fifo, FIFO_PATH

        while not self.shutdown_event.is_set():
            time.sleep(0.025)
            try:
                if(fifo == -1):
                    fifo_file = open(FIFO_PATH, 'r+')
                    fifo_file.truncate()
                    fifo_file.close()
                    fifo = os.open(FIFO_PATH, os.O_RDWR | os.O_NONBLOCK)
                    print('[fifo alert-'+str(fifo)+'] fifo is opened! path__'+FIFO_PATH)
                    break
                else :
                    print('[fifo alert-'+str(fifo)+'] fifo is already opened...')
                    break
            except:
                raise
                print("[fifo error] Can't open the fifo! path__" + FIFO_PATH)
                continue


    def run(self):
        global fifo, FIFO_PATH
        while not self.shutdown_event.is_set():
            try:
                self.openFifo()
                self.readFifo()
            except:
                raise
                print('[fifo stopped] there are error in fifo process\n\t\tJust give me a secs')
                time.sleep(3)
                if fifo != -1:
                    os.close(fifo)
                    fifo = -1
                    os.remove(FIFO_PATH)
                continue

class TimerThread(threading.Thread):
    
    def __init__(self, conf):
        threading.Thread.__init__(self)
        self.shutdown_event = threading.Event()
        self.conf = conf

    def run(self):
        while not self.shutdown_event.is_set():
            #adjust timer
            try:
                conf['timer'] += 1
                print(str(conf['timer']))
                if conf['timer'] > conf['timerTh']:
                    conf['timer'] = 0
                time.sleep(1)
            except:
                conf['timer'] = 0
                pass

class KeyboardThread(threading.Thread):
    
    def __init__(self, conf):
        threading.Thread.__init__(self)
        self.shutdown_event = threading.Event()
        self.conf = conf

    def proveKey(self, conf, key) :

        if conf['prevKey'] == key and conf['timer'] < conf['timerTh'] :
            conf['metric'] += getAcc()
        else :
            initBase()
            conf['metric'] = 2
            conf['prevKey'] = key
        conf['timer'] = 0

    def run(self):
        global fifo, FIFO_PATH, q_key, th_fifo, th_timer
        while not self.shutdown_event.is_set():
            '------------> ki yoon waitKey(argu) > the number of argu very very many,  we are keyboard ASCII surround, 0xFF = 256(ASCII num)'
            key = q_key.get()

            if key != 255 :
                print(str(key))
                #take branched process
                if key == ord("q"):
                    if fifo < 0:
                        with open(FIFO_PATH, 'w+') as fifo_file:
                            fifo_file.close()
                    th_timer.shutdown_event.set()
                    self.shutdown_event.set()
                    th_fifo.shutdown_event.set()
                    th_fifo.join()
                    cv2.destroyAllWindows()

                elif key == ord("w"):
                    #FIX IT BELOW
                    self.proveKey(conf, key)
                    #END FIX IT BELOW`

                    #DO NOT TOUCH
                    self.conf['y'] -= self.conf['metric']
                    if self.conf['y'] < self.conf['MIN_Y']:
                        self.conf['y'] = self.conf['MIN_Y']
                    #END DO NOT TOUCH

                elif key == ord("s"):
                    self.proveKey(conf, key)
                    self.conf['y'] += self.conf['metric']
                    if self.conf['y'] > self.conf['MAX_Y']:
                        self.conf['y'] = self.conf['MAX_Y']

                elif key == ord("d"):
                    self.proveKey(conf, key)
                    self.conf['x'] += self.conf['metric']
                    if self.conf['x'] > self.conf['MAX_X']:
                        self.conf['x'] = self.conf['MAX_X']
                    
                elif key == ord("a"):
                    self.proveKey(conf, key)
                    self.conf['x'] -= self.conf['metric']
                    if self.conf['x'] < self.conf['MIN_X']:
                        self.conf['x'] = self.conf['MIN_X']

th_timer = TimerThread(conf)
th_keyINput = KeyboardThread(conf)
th_fifo = FifoThread()

if __name__ == "__main__":

    th_timer.start()
    th_keyINput.start()
    th_fifo.start()

    for frame in camera.capture_continuous(rawCapture, format='bgr', use_video_port=True):
        #get source from camera
        image = frame.array

        '''pre-process'''
        #set text on the source
        cv2.putText(image
                    , 'x: ' + str(conf['x']) + ' y: ' + str(conf['y'])
                    ,topLeftCornerOfText
                    ,font
                    ,fontScale
                    ,fontColor
                    ,lineType)
        #set target on the source
        cv2.rectangle(image
                    ,(conf['x'],conf['y'])
                    ,(conf['x']+conf['w'], conf['y']+conf['h'])
                    ,(0,0,255)
                    ,1)
        #show source which is processed
        cv2.imshow("Origin", image)

        '''dealing process'''
        #crop that
        image = image[conf['y']:conf['y']+conf['h'], conf['x']:conf['x']+conf['w']]

        #show it
        cv2.imshow("Test", image)

        q_key.put(cv2.waitKey(1) & 0xFF)
        
        '''post-process'''
        #make clean the buffer above
        rawCapture.truncate(0)

        if(not th_keyINput.isAlive()):
            break