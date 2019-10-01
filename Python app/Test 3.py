import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style

class SubPlot(animation.TimedAnimation):
    
    def __init__(self, fig, (ax1, ax2)):
        self.x = np.array(0)
        self.y1 = np.array(0)
        self.y2 = np.array(0)

        fig, (ax1, ax2) = plt.subplots(1, 2)
        # rcParams is a dictionary that stores plot configuration
        plt.rcParams['font.size'] = 10
        plt.rcParams['font.style'] = 'italic'
        plt.rcParams['font.family'] = 'cursive'
        plt.rcParams['font.cursive'] = 'Textile'
        plt.rcParams['figure.figsize'] = [10, 5]        # I don't know what it does yet
        plt.rcParams['image.aspect'] = 'equal'
        self.UpdateAx1()
        self.UpdateAx2()
        animation.TimedAnimation.__init__(self, fig, interval=50, blit=True)
    
    def UpdateAx1(self):
        ax1.clear()
        ax1.set(title='Datos 1')
        ax1.grid(
            b='True',
            linestyle='--',
            color='#E6E6E6',
            alpha=0.5)
        ax1.set_xscale('linear', fontname='Times New Roman')
        ax1.set_yscale('linear')
        ax1.set_xlabel('Sample number')
        ax1.set_ylabel('Decimal Value')
        ax1.plot(_x[-20:-1], buffer[-20:-1],
            label='Raw',
            color='#3B8BEB',
            linewidth=2,
            markersize=2)
        ax1.legend(loc=1,
            facecolor='#F8E9A1',
            framealpha=0.5,
            edgecolor='#F3D250',
            mode='Expand')

    def UpdateAx2(self):
        ax2.clear()
        ax2.set(title='Datos 2')
        ax2.grid(b='True', linestyle='--', color='#E6E6E6', alpha=0.5)
        ax2.set_xscale('linear')
        ax2.set_yscale('linear')
        ax2.set_xlabel('Sample number')
        ax2.plot(_x[-20:-1], buffer2[-20:-1],
            label='Processed',
            color='#F13C20',
            linewidth=2,
            markersize=2)
        ax2.legend(loc=1,
            facecolor='#F8E9A1',
            framealpha=0.5,
            edgecolor='#F3D250')
    def Animate(self):
        ani = animation.FuncAnimation(fig, refresh_graph_data, len(x),
            fargs=[x],
            interval=500,
            blit=False)
        plt.tight_layout()
        plt.show()

Plot Martin
 = np.linspace(0,2*np.pi,10)
y1 = np.sin(x)
y2 = np.cos(x)
