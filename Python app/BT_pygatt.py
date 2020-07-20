from binascii import hexlify, unhexlify
import pygatt
import logging

from pygatt.util import uuid16_to_uuid

logging.basicConfig()
logging.getLogger('pygatt').setLevel(logging.DEBUG)

MACADDRESS = "7c:ec:79:69:62:f8"
SERVICE = "0000ffe1-0000-1000-8000-00805f9b34fb"
adapter = pygatt.BGAPIBackend(serial_port="COM15")


def byte2int(handle, bytevalue):
    """
    handle -- integer, characteristic read handle the data was received on
    value -- bytearray, the data returned in the notification
    returns -- list of integers
    """
    data = hexlify(bytevalue)
    #Stores the number of bytes sent by user...
    databytes = len(bytevalue)
    #...and use this info to slice and obtain decoded information
    intvalue = []
    for i in range(databytes):
        x = int(data[2*i : 2*(i+1)], 16)
        intvalue.append(x)
    print(intvalue)
    return intvalue

adapter.start()
device = adapter.connect(MACADDRESS)
characteristics = device.discover_characteristics()

#This is how we suscribe to receive data
device.subscribe(SERVICE, callback=byte2int)
device.unsubscribe(SERVICE)


device.char_write(value=hexlify(b"1"), uuid=SERVICE)
#I really have no clue how to use this properly
device.char_read(SERVICE)
device.disconnect()
adapter.stop()
