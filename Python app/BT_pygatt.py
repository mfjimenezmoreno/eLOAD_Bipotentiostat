import pygatt

MACADDRESS = "7c:ec:79:69:62:f8"
SERVICE = "0000ffe0-0000-1000-8000-00805f9b34fb"
adapter = pygatt.BGAPIBackend(serial_port="COM15")

try:
    adapter.start()
    device = adapter.connect(MACADDRESS)
    characteristics = device.discover_characteristics()
    for key in characteristics.keys():
        print(str(key) + ": " + str(characteristics[key]))
    SERVICE = "0000ffe1-0000-1000-8000-00805f9b34fb"
    device.char_write(value=[1],uuid=SERVICE)
    value = device.char_read(SERVICE)
finally:
    adapter.stop()
