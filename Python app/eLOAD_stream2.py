from functools import partial
import time
import numpy as np

from concurrent.futures import ThreadPoolExecutor
from tornado import gen
from threading import Thread

from bokeh.document import without_document_lock
from bokeh.models import ColumnDataSource
from bokeh.plotting import curdoc, figure

#this is the datasource for plotting
source = ColumnDataSource(data=dict(x=[0], y=[0], color=["blue"]),)

i = 0
x = np.empty(1)
y = np.empty(1)
doc = curdoc()

executor = ThreadPoolExecutor(max_workers=2)

plot = figure(x_range=[0, 10], y_range=[0, 5])

l = plot.circle(x='x', y='y', color='color', source=source)


def Main_Thread(i):
    time.sleep(1)

    return i

# the unlocked callback uses this locked callback to safely update
@gen.coroutine
def task_Redraw_update(i):

    source.stream(dict(x=[source.data['x'][-1]+1], y=[i], color=["blue"]))

#Here you input the blocking computation, won't prevent the other callbacks
#from execution, the decorator let's Bokeh know this
@gen.coroutine
@without_document_lock
def task_Redraw():
    global i
    i += 1
    res = yield executor.submit(Main_Thread, i)
    doc.add_next_tick_callback(partial(task_Redraw_update, i=res))  #All actions that update the document have to go through this callback


@gen.coroutine
def task_Process():
    #plot.x_range(source.data['x'][0], source.data['x'][-1])
    source.stream(dict(x=[source.data['x'][-1]+1], y=[i], color=["red"]),rollover=100)
    

def Main():
    global x
    global y
    x_init = 10E-9*time.time_ns()
    while True:
        # do some blocking computation
        x_transcurred = 10E-9*time.time_ns() - x_init
        y_function = np.sin(x/6)
        np.append(x, x_transcurred)
        np.append(y, y_function)
        time.sleep(0.05)



doc.add_periodic_callback(task_Redraw, 1000)
doc.add_periodic_callback(task_Process, 200)

doc.add_root(plot)

thread = Thread(target=Main)
thread.start()
