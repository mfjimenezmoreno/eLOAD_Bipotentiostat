from bokeh.io import output_file, show
from bokeh.plotting import figure
from bokeh.layouts import column
from bokeh.models.widgets import Button, RadioButtonGroup, Select, Slider

output_file("layout_widgets.html")

#Widget creation example
slider = Slider(start=0,end=10,step=1,value=1,title='Slider')
buttongroup = RadioButtonGroup(labels=['Option 1', 'Option 2', 'Option 3'], active=0)
select = Select(title='Option', value='foo', options=['foo', 'bar', 'glam', 'bam'])
button1 = Button(label='Button 1')
button2 = Button(label='Button 2')

show(column(button1, slider, buttongroup, select, button2, width=200))