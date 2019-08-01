#/////////////////////////////////#
#////////////Libraries////////////#
#/////////////////////////////////#
import time
import serial
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style

#/////////////////////////////////#
#////Plot related declarations////#
#/////////////////////////////////#
style.use('fivethirtyeight')
fig = plt.figure()
ax1 = fig.add_subplot(121)
ax2 = fig.add_subplot(122)

#/////////////////////////////////#
#////////////Variables////////////#
#/////////////////////////////////#
x = np.linspace(0, 19, 20)               #Horizontal vector of ten elements
buffer = np.zeros(20)                   #Data is stored here
buffer2 = np.zeros(20)                  #Proceced data is stored here
eLOAD = serial.Serial()                 #Port related configuration
eLOAD.baudrate = 9600
eLOAD.port = 'COM11'
eLOAD.timeout = 1
eLOAD.open()

#/////////////////////////////////#
#////////////Functions////////////#
#/////////////////////////////////#
def refresh_graph_data(i, _x):
    global buffer, buffer2
    print("Refreshing data...")
    
    for x in range(500):
        # Read a new line until terminator and turn it into string
        data = str(eLOAD.readline())

        if (data.__contains__("!") == False):
                # Identifies character '!', if its not there, bypass the rest of loop
                continue

        data = data[3:-5]                               # Get rid of initial and two final characters
        print(data)
        buffer = np.roll(buffer, 1)                     # Rotate data vectors and...
        buffer2 = np.roll(buffer2, 1)
        buffer[0] = int(data, 16)                       # ...store recent readout as integer (Performs conversion from HEX string)
        buffer2[0] =   buffer[0] - 2**32                # ...proper modification?
        _x += 1
        print(_x)
        print(buffer)
        print(buffer2)
        #Update plots
        ax1.clear()
        ax1.plot(_x[-20:-1], buffer[-20:-1])
        ax2.clear()
        ax2.plot(_x[-20:-1], buffer2[-20:-1])
        break

#/////////////////////////////////#
#//////////////Main///////////////#
#/////////////////////////////////#

ani = animation.FuncAnimation(fig, refresh_graph_data, len(x), fargs = [x], interval = 500, blit = False)
plt.show()
eLOAD.close()                                   # Close communication port
