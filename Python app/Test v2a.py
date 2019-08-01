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

# Create figure
fig, (ax1, ax2) = plt.subplots(1, 2)
plt.rcParams['font.size'] = 10                  # rcParams is a dictionary that stores plot configuration
plt.rcParams['font.style'] = 'italic'
plt.rcParams['font.family'] = 'cursive'
plt.rcParams['font.cursive'] = 'Textile'
plt.rcParams['figure.figsize'] = [10, 5]        # I don't know what it does yet
plt.rcParams['image.aspect'] = 'equal'

#/////////////////////////////////#
#////////////Variables////////////#
#/////////////////////////////////#
x = np.linspace(0, 499, 500)            # Horizontal vector of 500 elements
buffer = np.zeros(500)                  # Streamed data is stored here
buffer2 = np.zeros(500)                 # Processed data is stored here for experimental reasons
timestamp = [0, 0]                      # List variable used to determine time between events [t0 t-1], used to debug acquisiton time

#/////////////////////////////////#
#////////////Functions////////////#
#/////////////////////////////////#

#This is the animation function
def refresh_graph_data(i, _x):
    global buffer, buffer2, timestamp   # Must declare that these variables have global scope
    print("Refreshing data...")
    
    for x in range(500):                                # Note: Experiment maybe by removing this, the idea is not to refresh animation that fast
        # Read a new line until /n terminator and turn it into string
        data = str(eLOAD.readline())
        print(data)                                     # Pristine data print

        if (data.__contains__("!") == False):
                # If "!" is not there, skip the loop
                continue
        # 
        timestamp[1] = timestamp[0]                     # Save in 1-index the previous value at 0-index
        timestamp[0] = 10E-9*time.time_ns()             # Acquire time

        # Print the pristine data without useless characters
        print('Acquired = '+ data)
        data = data[3:-5]                               
        print(data)
        
        # Rotate the data at buffer 1 and 2, save the recent data into buffer[0]
        buffer = np.roll(buffer, 1)                     # Rotate data vectors and...
        buffer2 = np.roll(buffer2, 1)
        buffer[0] = int(data, 16)                       # ...store recent readout as integer (Performs conversion from HEX string)
        
        print(buffer[0])                                # Im just printing this to confirm 
        print(0xFF000000)
        print(buffer[0] > 0xFF000000)
        #Change the negative number to python's signed format, by checking if the 4th byte has FF
        if buffer[0] > 0xFF000000:                      #One easy way to acheive this, without the need of masks or bitshifts
                buffer2[0] = buffer[0] - 2**32          #Adjust it to be python friendly
        else:
                buffer2[0] = buffer[0]                  #Esle it should be positive and no further modification needed
        
        #Convert to voltage read at the transimpedance amplifier and its associated current
        pga_value = 2.0
        v_ref = 1.5
        gain = 3E3
        readout_volts = 2*v_ref*buffer2[0]/(pga_value*(2**24-1))
        readout_current = (v_ref-readout_volts)/gain
        _x += 1
        print(str(_x[0]))
        print('Pristine data\t'+data+'\t'+str(buffer[0])+'\t'+str(
            readout_volts)+' volts'+'\t'+str(readout_current)+' amps'+'\t'+str(timestamp[1]-timestamp[0]))
        print('2s conversion\t'+data+'\t'+str(buffer2[0])+'\t'+str(
            readout_volts)+' volts'+'\t'+str(readout_current)+' amps'+'\t'+str(timestamp[1]-timestamp[0]))
        #Update plots
        ax1.clear()
        ax1.set(title='Datos 1')
        ax1.grid(b='True', linestyle='--', color='#E6E6E6', alpha=0.5)
        ax1.set_xscale('linear', fontname='Times New Roman')
        ax1.set_yscale('linear')
        ax1.set_xlabel('Sample number')
        ax1.set_ylabel('Decimal Value')
        ax1.plot(_x[-20:-1], buffer[-20:-1], label='Raw', color='#3B8BEB',
                 linewidth=2, markersize=2)
        ax1.legend(loc=1, facecolor='#F8E9A1', framealpha=0.5,
                   edgecolor='#F3D250', mode='Expand')
        ax2.clear()
        ax2.set(title='Datos 2')
        ax2.grid(b='True', linestyle='--', color='#E6E6E6', alpha=0.5)
        ax2.set_xscale('linear')
        ax2.set_yscale('linear')
        ax2.set_xlabel('Sample number')
        ax2.plot(_x[-20:-1], buffer2[-20:-1], label = 'Processed', color = '#F13C20',
                linewidth=2, markersize=2)
        ax2.legend(loc=1, facecolor='#F8E9A1',
                   framealpha=0.5, edgecolor='#F3D250')
        break


#/////////////////////////////////#
#//////////////Main///////////////#
#/////////////////////////////////#

# Step 1: Configure and open Serial port
eLOAD = serial.Serial()                 # eLOAD is how I will call the serial port
eLOAD.baudrate = 9600
# WARNING: change this according to your computer
eLOAD.port = 'COM11'
eLOAD.timeout = 1                       # How much time to wait in ms
eLOAD.open()

#Step 2: Animate by calling refresh function
ani = animation.FuncAnimation(fig, refresh_graph_data, len(x), fargs = [x], interval = 500, blit = False)
plt.tight_layout()
plt.show()
eLOAD.close()                                   # Close communication port
