from bokeh.io import curdoc
from bokeh.plotting import figure, show
from bokeh.models import CustomJS, ColumnDataSource
from bokeh.models.widgets import Select, Button, Slider, Paragraph, TextInput, TableColumn, DataTable, TableColumn, Toggle
from bokeh.layouts import gridplot, column, row
from bokeh.events import ButtonClick
from bokeh.driving import count
from bokeh.document import without_document_lock
import numpy as np
from concurrent.futures import ThreadPoolExecutor
from tornado import gen
from eLoaD_functions import rollDict, clearDict

#----------------------#
#    Initialization    #
#----------------------#
"""Global Variables"""
eLoad_Connection = False
eLoad_COM = 'COM13'

"""Widgets configuration"""
Gain = Select(title="Gain", options=['100', '3k', '30k', '300k', '3M', '30M'], value='30k')
Connect = Button(label='Connect')
Random_test = Button(label='Random',button_type='warning')
Start = Button(label='Start', button_type='success')
Voltage = Slider(start=-1.5, end=1.5, value=1, step=0.01, title='Voltage')
Comm_Status_Message = Paragraph(text="Status: Not connected")
Port_input = TextInput(title='Port:', value='COM13')
Save = Button(label='Save', button_type='success')

"""Max threadpool executors"""
executor = ThreadPoolExecutor(max_workers=2)

#---------------------#
#    Figure Config    #
#---------------------#
"""Dataframe structure"""
transferData = {'time':[],
                'raw_data':[],
                #'current':[]
                }
acquiredData = {'timestamp':[],
                'raw_data':[]}

source = ColumnDataSource(data=transferData)

"""Table for data visualization"""
columns = [
    TableColumn(field='time', title='X'),
    TableColumn(field='raw_data', title='Y')
]
Table = DataTable(source=source, columns=columns, width=400, height=280)

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


#-------------------------------#
#    Callbacks (Python side)    #
#-------------------------------#
#Callbacks address which data is sent to Bokeh server

def callback_Connect_to_eLoaD():
    """Still requires modification, to verify the port"""
    global eLoad_Connection
    global eLoad_COM

    if eLoad_Connection is True:
        Comm_Status_Message.text = "Status: Not connected"
        eLoad_Connection = False
    else:
        Comm_Status_Message.text = "Status: Connected"
        eLoad_Connection = True
    print(eLoad_Connection)

#------------------------------#
#    Callbacks (Server Side)   #
#------------------------------#
#Server react to data as sent by Python, defined by JS code.

#Deprecated
callback_status = CustomJS(args=dict(Port=Port_input, Message=Comm_Status_Message, conStatus = eLoad_Connection), 
                                     code="""
    
    if(Message.text.split(': ')[1]=="Not connected"){
        Message.text = "Status: Connected";
        conStatus = true;
    }
    else{
        Message.text = "Status: Not connected";
        conStatus = false;
    }
    
    conStatus.change.emit();
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

def callback_Start(new):
    global eLoad_Connection
    global Start
    if eLoad_Connection is False:
        print("eLoaD not connected")
        Start.label = "Start"           #Overkill
    else:
        print("eLoad is connected")
        Start.label = "Stop"
    

def callback_Random():
    global eLoad_Connection
    
    if eLoad_Connection is True:
        Comm_Status_Message.text = "Status: Not connected"
        eLoad_Connection = False
    else:
        Comm_Status_Message.text = "Status: Connected"
        eLoad_Connection = True
    print(eLoad_Connection)

"""Callback Assignments"""
Connect.on_click(callback_Connect_to_eLoaD)
Save.js_on_click(callback_Save)
Start.on_click(callback_Start)
Random_test.on_click(callback_Random)

#This has priority, locked task
#@gen.coroutine
@count()
def acquire_data(t):
    global acquiredData
    new_x = np.random.uniform(-1, 1, size=5)
    new_y = np.random.uniform(-10, 10, size=5)
    new=dict(timestamp=new_x,raw_data=new_y)       #Format to roll into azquiredData dictionary
    rollDict(new, acquiredData, 200)   #Roll new data into acquiredData, rollover of 1000
    print("Acquire data")
    print(len(acquiredData['timestamp']))
    #print(acquiredData)
#Plotting has no priority, also slower
#@gen.coroutine
#@without_document_lock
@count()
def update_plot(t):
    global acquiredData
    source.stream({'time':acquiredData['timestamp'],'raw_data':acquiredData['raw_data']}, 500)
    clearDict(acquiredData)
    print("Update plot")
    print(len(transferData['time']))

#-----------#
#    GUI    #
#-----------#
"""Front End"""
Comm_Panel = row(Port_input, Comm_Status_Message)
Panel = row(column(Comm_Panel, Connect, Gain, Voltage, Start, Save, Random_test),plot_raw,plot_current, Table)


curdoc().add_periodic_callback(update_plot,3000)
curdoc().add_periodic_callback(acquire_data,1000)
curdoc().add_root(Panel)
"""
Lists of things to do now:
    -Generate random data, plot it
    -Make sure it plots only recent data for the mean time
    -Solve multithreading strategy
    -Acquire data form eLoaD
"""
