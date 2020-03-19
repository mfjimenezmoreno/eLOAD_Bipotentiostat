import numpy as np
import eLoaD_functions as Nane
from bokeh.io import curdoc
from bokeh.plotting import figure, show
from bokeh.models import CustomJS, ColumnDataSource
from bokeh.models.widgets import Select, Button, Slider, Paragraph, TextInput, TableColumn, DataTable, TableColumn, Toggle
from bokeh.layouts import gridplot, column, row
from bokeh.events import ButtonClick
from bokeh.driving import count
from bokeh.document import without_document_lock
import pandas as pd

lista = ['FFFFFFFF','00000000','00000001','7FFFFFFF','80000000']

Nane.rawHexString_to_int('FFFFFFFF', bits=24)
Nane.raw_to_Volts('FFFFFFFF', pga=2, v_ref=1.5)
Nane.raw_to_current('002AAAAA', pga=2, muxGain=3000, v_ref=1.5)

CV = pd.read_csv(r'Cap.csv')
CV.columns = ['Voltage', 'Current']
CV['Voltage'].iloc[3]
source = ColumnDataSource(data=CV)

plot_raw = figure(title='Voltage vs. Current',
                  plot_height=500, plot_width=500)
plot_raw.toolbar_location = None
plot_raw.x_range.follow = 'end'
plot_raw.x_range.follow_interval = 100
plot_raw.x_range.range_padding = 0.1
plot_raw.circle(source=source, x='Voltage', y='Current', alpha=0.2, size=5,
                line_width=3, color='#FF0000')

show(plot_raw)
