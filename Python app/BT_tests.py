import bluetooth

target_name = "My Phone"
target_address = None

nearby_devices = bluetooth.discover_devices()
nearby_devices
for bdaddr in nearby_devices:
    print(bdaddr)
    if target_name == bluetooth.lookup_name(bdaddr):
        target_address = bdaddr
        break

if target_address is not None:
    print("found target bluetooth device with address ", target_address)
else:
    print("could not find target bluetooth device nearby")
pass
#TODO:
#Choosing a device to communicate with.
#Figure out how to comunnicate with it.
#Making an outgoing connection.
#Accepting an incoming connection.
#Sending data.
#Receiving data.

#DONE:

#FIXME
