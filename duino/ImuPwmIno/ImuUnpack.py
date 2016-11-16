import serial
import struct

ser = serial.Serial(timeout=1)
ser.port = 'COM8'
ser.baudrate = 38400
ser.parity=serial.PARITY_NONE
ser.stopbits=serial.STOPBITS_ONE
ser.bytesize=serial.EIGHTBITS
ser.open()
line=ser.readline()

for i in range(0,10):
    line=ser.readline()
    if "I:" in line:
        print struct.unpack('Lhhhhhhhhh',line[2:-2])
    if "IMU" in line:
        print line[0:-1]
        leng=len(line)

print leng
'''
while "IMU:" not in line:
    line=ser.readline()

print line
print len(line)
print line[3:-1]
print len(line[4:-2])
print struct.unpack('Lhhhhhhhhh',line[4:-2])
'''