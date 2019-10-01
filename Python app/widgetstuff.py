from bokeh.io import output_file, show
from bokeh.layouts import column, gridplot
from bokeh.plotting import figure

output_file("layout.html")

x = list(range(11))
y0 = x
y1 = [10 - i for i in x]
y2 = [abs(i-5) for i in x]

#Create plot
s1 = figure(plot_width=250, plot_height=250, title=None)
s1.circle(x, y0, size=10,color='navy',line_alpha=0.5)

s2 = figure(plot_width=250, plot_height=250, title=None)
s2.circle(x, y1, size=10,color='firebrick',line_alpha=0.5)

s3 = figure(plot_width=250, plot_height=250, title=None)
s3.circle(x, y2, size=10, color='olive', line_alpha=0.5)

#Assign plots to a column position
#show(column(s1, s2, s3))

#Gridlike
grid = gridplot([[s1,s2],[s2,s3]])
show(grid)