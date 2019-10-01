from bokeh.plotting import figure, show

x_values = [1,2,3,4,5]
y_values = [6,7,8,9,10]

p=figure()
p.circle(x=x_values,y=y_values)

show(p)