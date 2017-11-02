import io
import random
import picamera
from PIL import Image
from moviepy.editor import *
import moviepy.video.fx.all as vfx 

'''
prior_image = None

def detect_motion(camera):
    global prior_image
    stream = io.BytesIO()
    camera.capture(stream, format='jpeg', use_video_port=True)
    stream.seek(0)
    if prior_image is None :
        prior_image = Image.open(stream)
        return False
    else :
        current_image = Image.open(stream)
        result = random.randint(0, 10) == 0
        prior_image = current_image
        return result

def write_video(stream) :
    with io.open('before.h264', 'wb') as output :
        for frame in stream.frames:
            if frame.frame_type == picamera.PiVideoFrameType.sps_header:
                stream.seek(frame.position)
                break
        while True:
            buf = stream.readl()
            if not buf:
                break
            output.write(buf)
        stream.seek(0)
        stream.truncate()
'''

'''start HERE'''
with picamera.PiCamera() as camera:
    camera.resolution = (1280, 720)
    stream = picamera.PiCameraCircularIO(camera, seconds=10)
    camera.start_recording(stream, format='h264')

    clip = (VideoClip(stream)
    .subclip((1,10.1), (1,14.9))
    .resize(0.5)
    .crop(x1=145, y1=110, x2=400, y2=810)
    )
    new_clip = vfx.scroll(clip, x_speed=1)

    try:
        while True:
            camera.wait_recording(1)
            new_clip.write_videofile('newone.h264')
            '''camera.wait_recording(1)
            if detect_motion(camera):
                print('Motion detected!')
                camera.split_recording('after.h264')
                write_video(stream)
                while detect_motion(camera):
                    camera.wait_recording(1)
                print('Motion stopped!')
                camera.split_recording(stream)'''
    finally:
            camera.stop_recording()
