from bokeh.io import curdoc
from bokeh.plotting import figure
from bokeh.models import ColumnDataSource
from random import random

source_ios = ColumnDataSource({'Date': [], 'Vol': []})

ios = figure(toolbar_location=None,plot_width=800, plot_height=250)
ios.xaxis.axis_label = 'Date'
ios.yaxis.axis_label = 'Vol'
ios.min_border_left = 50
ios.min_border_bottom = 50

ios.circle(x='Date',y='Vol',color="pink", size=20, source=source_ios)

i=0
def update():
    global i
    new_MAU_ios = {'Date':range(i,i+10),'Vol':[random() for j in range(10)]}
    source_ios.stream(new_MAU_ios)
    i+=10

curdoc().add_periodic_callback(update, 800)

curdoc().add_root(ios)