import matplotlib.pyplot as plt
import matplotlib as mlt
import numpy as np
import math

x = np.linspace(0,6*math.pi,100)
y = np.sin(x)
y2 = np.cos(x)

fig = plt.figure()
ax1 = fig.add_subplot(121)
ax2 = fig.add_subplot(122)


fig.axes(facecolor = 'w', ylim = (-2 , 2))

fig.grid(b = 'True', linestyle = '--', color = '#E6E6E6')
ax1.plot(x, y)
ax2.plot(x, y2)

plt.show()
