from binascii import hexlify
import pygatt
import logging


MACADDRESS = "7c:ec:79:69:62:f8"
SERVICE = "0000ffe1-0000-1000-8000-00805f9b34fb"
adapter = pygatt.BGAPIBackend(serial_port="COM15")


def handle_data(handle, value):
    """
    handle -- integer, characteristic read handle the data was received on
    value -- bytearray, the data returned in the notification
    """
    print("Received data: %s" % hexlify(value))

adapter.start()
device = adapter.connect(MACADDRESS)
characteristics = device.discover_characteristics()
device.subscribe(SERVICE, callback=handle_data)

device.char_write(hexlify("01"),uuid=SERVICE)
value = device.char_read(SERVICE)
adapter.stop()
