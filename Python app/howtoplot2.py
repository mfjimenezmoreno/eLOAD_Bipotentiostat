from bokeh.plotting import figure, show
from bokeh.models import ColumnDataSource

data = {'x_values':[1,2,3,4,5],
        'y_values':[6,7,8,9,10]}

source = ColumnDataSource(data=data)

p=figure()
p.circle(source=source,x='x_values',y='y_values')

show(p)