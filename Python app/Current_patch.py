from bokeh.io import curdoc
from bokeh.plotting import figure, show
from bokeh.models import CustomJS, ColumnDataSource
from bokeh.models.widgets import Select, Button, Slider, Paragraph, TextInput
from bokeh.layouts import gridplot, column, row
from bokeh.events import ButtonClick
import numpy as np

#----------------------#
#    Initialization    #
#----------------------#
"""Global Variables"""
eLoad_Connection = False
eLoad_COM = 'COM13'

"""Widgets configuration"""
Gain = Select(title="Gain", options=['100', '3k', '30k', '300k', '3M', '30M'], value='30k')
Connect = Button(label='Connect')
Start = Button(label='Start', button_type='success')
Voltage = Slider(start=-1.5, end=1.5, value=1, step=0.01, title='Voltage')
Comm_Status_Message = Paragraph(text="Status: Not connected")
Port_input = TextInput(title='Port:', value='COM13')
Save = Button(label='Save', button_type='success')

#---------------------#
#    Figure Config    #
#---------------------#
"""Dataframe structure"""
transferData = {'time':[i for i in range(100)],
                'raw_data':[2*i for i in range(100)]}

source = ColumnDataSource(data=transferData)

"""Plot related"""
plot_raw = figure(title='Raw Data',
                  plot_height=500, plot_width=500)
plot_raw.toolbar_location = None
plot_raw.x_range.follow = 'end'
plot_raw.x_range.follow_interval = 100
plot_raw.x_range.range_padding = 0
plot_raw.line(source=source, x='time', y='raw_data', alpha=0.2,
              line_width=3, color='navy')

plot_current = figure(title='Current',
                      plot_width=500,plot_height=500)
plot_current.toolbar_location = None
plot_current.x_range.follow = 'end'
plot_current.x_range.follow_interval = 100
plot_current.x_range.range_padding = 0

#-----------------#
#    Callbacks    #
#-----------------#
"""Callback section"""
callback_status = CustomJS(args=dict(Port=Port_input, Message=Comm_Status_Message), code="""

    if(Message.text.split(': ')[1]=="Not connected"){
        Message.text = "Status: Connected";
    }
    else{
        Message.text = "Status: Not connected";
    }
    
""")

callback_Save = CustomJS(args=dict(source=source), code="""
    //Function acquires converts the source to a csv

    function table_to_csv(source){
        const columns = Object.keys(source.data);   //Acquire number of keywords!
        const nrows = source.get_length();          //How to obtain length...of data I guess
        const lines = [columns.join(',')];

        for(let i = 0; i < nrows; i++){
            let row = [];
            for(let j = 0; j < columns.length; j++){
                const column = columns[j];
                row.push(source.data[column][i].toString());
            }
            lines.push(row.join(','));
        }
        return lines.join('\\n').concat('\\n');
    }
    
    const filename = 'Data_result.csv';
    filetext = table_to_csv(source);
    const blob = new Blob([filetext],{ tpye: 'text/csv;charset=utf-8;' });
    
    //addresses IE
    if(navigator.msSaveBlob){
        navigator.msSaveBlob(blob, filename);
    }
    else{
        const link = document.createElement('a');
        link.href = URL.createObjectURL(blob);
        link.download = filename;
        link.target = '_blank';
        link.style.visibility = 'hidden';
        link.dispatchEvent(new MouseEvent('click'));
    }
""")

"""Callback Assignments"""
Connect.js_on_click(callback_status)
Save.js_on_click(callback_Save)

def update():
    s = slice(10)
    new_x = source.data['time'][s] + np.random.uniform(-0.1, 0.1, size=10)
    new_y = source.data['raw_data'][s] + np.random.uniform(-1, 1, size=10)
    source.patch({'time': [(s, new_x)], 'raw_data': [(s, new_y)]})

#-----------#
#    GUI    #
#-----------#
"""Front End"""
Comm_Panel = row(Port_input, Comm_Status_Message)
Panel = row(column(Comm_Panel, Connect, Gain, Voltage, Start, Save),plot_raw,plot_current)

#Show is the classical way, curdoc is the server one
#show(Panel)
curdoc().add_periodic_callback(update,500)
curdoc().add_root(Panel)
"""
Lists of things to do now:
    -Generate random data, plot it
    -Make sure it plots only recent data for the mean time
    -Solve multithreading strategy
    -Acquire data form eLoaD
"""
