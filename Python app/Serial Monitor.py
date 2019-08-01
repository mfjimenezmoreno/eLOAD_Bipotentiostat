#Libraries
import serial
from time import sleep
import sys

#Setup
COM = 'COM11'  # /dev/ttyACM0 (Linux)
BAUD = 9600

ser = serial.Serial(COM, BAUD, timeout=.1)

print('Waiting for device')
sleep(3)                                                #Delay for three seconds
print(ser.name)

#check args
if("-m" in sys.argv or "--monitor" in sys.argv):
	monitor = True
else:
	monitor = False

while True:
	# Capture serial output as a decoded string
	val = str(ser.readline().decode().strip('\r\n'))
	valA = val.split("/")
	#print()
	if(monitor == True):
		print(val, end="\r", flush=True)
