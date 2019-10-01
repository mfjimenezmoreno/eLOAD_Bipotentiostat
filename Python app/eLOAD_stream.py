from functools import partial
from random import random
from threading import Thread
import time
import numpy as np

from bokeh.models import ColumnDataSource
from bokeh.plotting import curdoc, figure
import bokeh.palettes
from bokeh.layouts import row

from tornado import gen

# this must only be modified from a Bokeh session callback
source = ColumnDataSource(data=dict(x=[0], y=[0]))

# This is important! Save curdoc() to make sure all threads
# see the same document.
doc = curdoc()

@gen.coroutine
def update(x, y):
    source.stream(dict(x=[x], y=[y]))
#------------------------------------------#
#This is the task that performs computation#
#------------------------------------------#
def Main_Task():
    x_init = 10E-9*time.time_ns()
    while True:
        # do some blocking computation
        time.sleep(0.1)
        x = 10E-9*time.time_ns() - x_init
        y = np.sin(x/6)

        # but update the document from callback
        doc.add_next_tick_callback(partial(update, x=x, y=y))

Pristine = figure(x_range=[0, 1], y_range=[0,1])
Pristine.circle(x='x', y='y', source=source)

doc.add_root(Pristine)

thread = Thread(target=Main_Task) #And this is how we assign a thread to the computation
thread.start()
