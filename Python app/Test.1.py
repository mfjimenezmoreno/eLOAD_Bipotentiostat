#/////////////////////////////////#
#////////////Libraries////////////#
#/////////////////////////////////#
import time
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
from math import pi

#/////////////////////////////////#
#////Plot related declarations////#
#/////////////////////////////////#
style.use('ggplot')
fig = plt.figure()
ax1 = fig.add_subplot(121)
ax2 = fig.add_subplot(122)

#/////////////////////////////////#
#////////////Variables////////////#
#/////////////////////////////////#
_x= np.linspace(0, 79, 40)               #Horizontal vector of ten elements
buffer1 = []
buffer2 = []

#/////////////////////////////////#
#////////////Functions////////////#
#/////////////////////////////////#
def refresh_graph_data(i, x, y1, y2):
    x += 1
    y1 = np.sin(x*pi/20)
    y2 = np.cos(x*pi/20)
    plt.ylim()
    ax1.clear()
    ax1.plot(x, y1)
    plt.title = "Data"
    plt.ylabel('Intensity')
    ax2.clear()
    ax2.plot(x, y2)
    ax2.plot()
    plt.title = "Data"
    plt.ylabel('Intensity')
    
    print(x)

#/////////////////////////////////#
#//////////////Main///////////////#
#/////////////////////////////////#
ani = animation.FuncAnimation(fig, refresh_graph_data, len(_x), fargs = [_x, buffer1, buffer2],interval = 500, blit = False)
plt.show()
print("Ya pasó plt.show")
