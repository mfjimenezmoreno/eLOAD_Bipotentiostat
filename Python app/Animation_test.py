from time import sleep
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style

style.use('fivethirtyeight')
fig = plt.figure()
ax1 = fig.add_subplot(121)
ax2 = fig.add_subplot(122)

def refresh_graph_data(i):
    print("Refreshing data...")
    graph_data = open("Datos.txt", "r").read()
    lines = graph_data.split("\n")
    xValues = []
    yValues = []
    for line in lines:
        if len(line) > 1:
            x, y = line.split(",")
            xValues.append(x)
            yValues.append(y)
    ax1.clear()
    ax1.plot(xValues, yValues)
    ax2.clear()
    ax2.plot(xValues, yValues)
    
ani = animation.FuncAnimation(fig, refresh_graph_data, interval = 1000)
plt.show()
print(lines)
while(1):
    sleep(3)
