from functools import partial
import time

from concurrent.futures import ThreadPoolExecutor
from tornado import gen

from bokeh.document import without_document_lock
from bokeh.models import ColumnDataSource
from bokeh.plotting import curdoc, figure
from bokeh.models.widgets import Button
from bokeh.layouts import column, row

source = ColumnDataSource(data=dict(x=[0], y=[0], color=["blue"]))

i = 0

doc = curdoc()

executor = ThreadPoolExecutor(max_workers=2)

def blocking_task(i):
    time.sleep(1.5)
    return i

@gen.coroutine
def locked_update(i):
    source.stream(dict(x=[source.data['x'][-1]+1], y=[i], color=["blue"]))

@gen.coroutine
@without_document_lock
def unlocked_task():
    global i
    i += 1
    res = yield executor.submit(blocking_task, i)
    doc.add_next_tick_callback(partial(locked_update, i=res))


@gen.coroutine
def update():
    source.stream(dict(x=[source.data['x'][-1]+1], y=[i], color=["red"]))
    print(source.data)


p = figure(x_range=[0, 100], y_range=[0, 20])
l = p.circle(x='x', y='y', color='color', source=source)

#---------------------------#
#          Buttons          #
#---------------------------#

Add_Callbacks = Button(label='Add Callbacks', button_type='success', width=320)
Remove_Callbacks = Button(label='Remove Callbacks',
                          button_type='warning', width=320)

callback_unlocked_task = None
callback_update = None

def callback_Add_threads():
    #NOTE: Adding callbacks from here, can't be removed
    global callback_unlocked_task
    global callback_update
    callback_unlocked_task = doc.add_periodic_callback(unlocked_task, 1000)
    callback_update = doc.add_periodic_callback(update, 200)

def callback_Remove_threads():
    #The following functions are not able to acces these threads
    doc.remove_periodic_callback(callback_unlocked_task)
    doc.remove_periodic_callback(callback_update)
    source.data = {'x':[0], 'y':[0], 'color':["blue"]}
    global i
    i = 0

Add_Callbacks.on_click(callback_Add_threads)
Remove_Callbacks.on_click(callback_Remove_threads)

#NOTE: Adding callbacks from here, can be removed by my button.
#callback_unlocked_task = doc.add_periodic_callback(unlocked_task, 1000)
#callback_update = doc.add_periodic_callback(update, 200)
doc.add_root(row(p, column(Add_Callbacks, Remove_Callbacks)))
