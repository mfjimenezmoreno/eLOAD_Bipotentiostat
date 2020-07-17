from bleak import BleakClient
import asyncio
from bleak import discover


async def run():
    devices = await discover()
    for d in devices:
        print("Nada")
        print(d)

loop = asyncio.get_event_loop()
loop.run_until_complete(run())


address = "7c:ec:79:69:62:f8"
MODEL_NBR_UUID = "00002a24-0000-1000-8000-00805f9b34fb"


async def run(address, loop):
    async with BleakClient(address, loop=loop) as client:
        model_number = await client.read_gatt_char(MODEL_NBR_UUID)
        print("Model Number: {0}".format("".join(map(chr, model_number))))

loop = asyncio.get_event_loop()
loop.run_until_complete(run(address, loop))
