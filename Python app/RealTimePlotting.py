import random

from bokeh.driving import count
from bokeh.models import ColumnDataSource
from bokeh.plotting import curdoc, figure

UPDATE_INTERVAL = 100
ROLLOVER = 100

source = ColumnDataSource({"x": [], "y": []})


@count()
def update(x):
    y = random.random()
    source.stream({"x": [x], "y": [y]}, rollover=ROLLOVER)


p = figure()
p.line("x", "y", source=source)

doc = curdoc()
doc.add_root(p)
doc.add_periodic_callback(update, UPDATE_INTERVAL)
