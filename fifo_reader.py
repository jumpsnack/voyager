import os
import errno
import time
import json

while True:
	try:
		fifo = os.open("face_detect_fifo", os.O_RDWR)
	except:
		raise
		print("Trouble!")
		time.sleep(1.25)
		continue

	print('FIFO opened')
	try:
		while True:
			time.sleep(0.02)
			data = os.read(fifo, 1024)
			if(len(data) != 0):
				print('Read: "{0}"'.format(data))
			else:
				time.sleep(1.25)
                print('None')
                '''
                try:
                    parsed_json = json.loads(data)
                    print('cnt: {0}, cx: {1}, cy: {2}, left: {3}, top: {4}, right: {5}, bottom: {6}'.format(parsed_json['cnt'], parsed_json['cx'], parsed_json['cy'], parsed_json['left'], parsed_json['top'], parsed_json['right'], parsed_json['bottom']))
                except:
                    pass'''
	except:
		raise
