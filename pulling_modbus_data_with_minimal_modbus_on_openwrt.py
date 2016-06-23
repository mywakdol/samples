
#!/usr/bin/env python

import minimalmodbus
import serial
import requests

# Change this serial settings to reflect yours
m = minimalmodbus.Instrument('/dev/ttyUSB0', 1) # port name, slave address (in decimal)
m.serial.baudrate = 9600
m.serial.bytesize = 8
m.serial.stopbits = 1
m.serial.parity   = serial.PARITY_NONE

# Check your power monitoring user guide for your device's register list
# Btw, I'm using Schneider PowerLogic Power Meter PM5350 
# and I can see Voltage A-N is on register 3028 and so on (index starts from 0)

voltage_AN = m.read_float(3027, 3, 2)
voltage_BN = m.read_float(3029, 3, 2)
voltage_CN = m.read_float(3031, 3, 2)

url='http://agnosthings.com/{GUID}/feed?push=v_an=' + str(voltage_AN) + ',v_bn=' + str(voltage_BN) + ',v_cn=' + str(voltage_AN)'

r = requests.get(url)
