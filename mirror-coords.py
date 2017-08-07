from vpython import *
import time
import threading

#scene.autoscale = False

print("\n")

hmd = vector(0,0,0)
left = vector(0,0,0)
right = vector(0,0,0)

devices = [hmd, left, right]

def process():
	xyz = ['x','y','z']
	global devices
	editIndex = -1;
	while True:
		try:
			nInput = input()
			splits = nInput.split()    		
		except Exception as e:
			break
		else:
			pass
		finally:
			pass
		for split in splits:
			if "HMD:" in split:
				editIndex = 0;
			if "LEFT:" in split:
				editIndex = 1;
			if "RIGHT:" in split:
				editIndex = 2;

			if editIndex < 0 or len(split) < 3 or split[1] != ':' or split[0] not in xyz:
				continue
			num = float(split[2:])
			if split[0] == 'x':
				devices[editIndex].z = -num;
			elif split[0] == 'y':
				devices[editIndex].y  = num;
			elif split[0] == 'z':
				devices[editIndex].x  = -num;


		
thread = threading.Thread(target=process)
thread.start()

hmd = box(color=color.black)
left = box(color=color.blue)
right = box(color=color.red)

box(color=color.white)

while True:
	hmd.pos = devices[0] * 2
	left.pos = devices[1] * 2
	right.pos = devices[2] * 2

