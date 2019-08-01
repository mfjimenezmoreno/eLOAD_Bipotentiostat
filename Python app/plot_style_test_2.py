import numpy as np
import matplotlib.pyplot as plt
import math

# Data for plotting
t = np.linspace(0,2*math.pi, 100)
y1 = np.sin(t)
y2 = np.cos(t)

# Create figure
fig, (ax1, ax2) = plt.subplots(1, 2)
plt.rcParams['font.size'] = 10
plt.rcParams['font.style'] = 'italic'
plt.rcParams['font.family'] = 'cursive'
plt.rcParams['font.cursive'] = 'Textile'
plt.rcParams['figure.figsize'] = [10, 5] #I don't know what id does
plt.rcParams['image.aspect'] = 'equal'
plt.rcParams.keys() #Ask this line to interpreter for a list of dictionary options

# linear y1 axis
ax1.plot(t, y1, label='Raw',color='#3B8BEB',linewidth=2,markersize=2)
ax1.set(title='Datos 1')
ax1.grid(b='True', linestyle='--', color='#E6E6E6', alpha=0.5)
ax1.set_xscale('linear', fontname='Times New Roman')
ax1.set_yscale('linear')
ax1.set_xlabel('Sample number')
ax1.set_ylabel('Decimal Value')
ax1.legend(loc=1, facecolor='#F8E9A1', framealpha=0.5, edgecolor='#F3D250', mode='Expand')
#ax1.set_aspect(10)


# linear x2 axis
ax2.plot(t, y2, label='Processed', color='#F13C20', linewidth=2, markersize=2)
ax2.set(title='Datos 2')
ax2.grid(b='True', linestyle='--', color='#E6E6E6', alpha=0.5)
ax2.set_xscale('linear')
ax2.set_yscale('linear')
ax2.set_xlabel('Sample number')
ax2.legend(loc=1, facecolor='#F8E9A1', framealpha=0.5, edgecolor='#F3D250')

plt.tight_layout()
plt.show()
