import serial 
import time 

ser = serial.Serial()
ser.port = "/dev/ttyUSB0"
ser.baudrate = 9600
ser.timeout = 0 

try: 
    ser.open()
    print "serial port is opened"

    str_list = []
    while True: 
        time.sleep(0.01)
        next_char = ser.read(ser.inWaiting())
        if next_char: 
            str_list.append(next_char)
        else: 
            if len(str_list) > 0:
                data = ''.join(str_list)
                data_efe = ":".join("{:02x}".format(ord(c)) for c in data)
                s = data_efe.split(':')
                data_x = (int('0x'+s[0], 16) << 8) + (int('0x'+s[1], 16))
                print data_efe, data_x

                str_list = []
except Exception as e :
    print "Exception: ", e.message 
    ser.close()

