from bokeh.io import curdoc
from bokeh.plotting import figure, show
from bokeh.models import CustomJS, ColumnDataSource
from bokeh.models.widgets import Select, Button, Slider, Paragraph, TextInput, TableColumn, DataTable, TableColumn, Toggle
from bokeh.layouts import gridplot, column, row
from bokeh.events import ButtonClick
from bokeh.driving import count
from bokeh.document import without_document_lock

import numpy as np
import pandas as pd
from concurrent.futures import ThreadPoolExecutor
from tornado import gen
import serial
import time
from eLoaD_functions import rollDict, clearDict

#----------------------#
#    Initialization    #
#----------------------#
"""Global Variables"""
eLoaD_Connection = False    #Used as a global flag, act according eLoaD connection
eLoaD_COM = 'COM13'         #Stores the serial port
eLoaD = serial.Serial()
eLoaD.baudrate = 9600
eLoaD.timeout = 1
MUX_Gain = 3E3      #For storing transimpedance gain as chosen by MUX
pga_value = 2.0     #The gain as specified by ADC's datasheet
v_ref = 1.5         #The reference voltage

"""Test data"""
CV = pd.read_csv(r'Cap.csv')
CV.columns = ['Voltage', 'Current']
source = ColumnDataSource(data=CV)

"""Widgets configuration"""
Gain = Select(title="Gain", options=['100', '3k', '30k', '300k', '3M', '30M'], value='30k')
Connect = Button(label='Connect')
Random_test = Button(label='Random',button_type='warning')
Start = Button(label='Start', button_type='success')
Voltage_Start = Slider(start=-1.5, end=1.5, value=0.2,
                       step=0.01, title='Voltage Start')
Voltage_Floor = Slider(start=-1.5, end=1.5, value=0.1,
                       step=0.01, title='Voltage Floor')
Voltage_Ceiling = Slider(start=-1.5, end=1.5, value=0.3,
                       step=0.01, title='Voltage Ceiling')
Comm_Status_Message = Paragraph(text="Status: Connected")
Port_input = TextInput(title='Port:', value='COM13')
Save = Button(label='Save', button_type='warning')

"""Max threadpool executors"""
executor = ThreadPoolExecutor(max_workers=3)

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
source_2 = ColumnDataSource(data=acquiredData)

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
plot_raw.x_range.range_padding = 0.1
plot_raw.xaxis.axis_label = "Voltage (Volts)"
plot_raw.yaxis.axis_label = "Current (Amperes)"
#plot_raw.line(source=source, x='time', y='raw_data', alpha=0.2,
#              line_width=3, color='navy')
plot_raw.circle(source=source, x='time', y='raw_data', alpha=0.2, size=10,
                line_width=3, color='#FF0000')


plot_current = figure(title='Current',
                      plot_width=500,plot_height=500)
plot_current.toolbar_location = None
plot_current.x_range.follow = 'end'
plot_current.x_range.follow_interval = 100
plot_current.x_range.range_padding = 0
plot_current.line(source=source, x='time', y='raw_data', alpha=0.2,
              line_width=3, color='navy')

#-------------------------------#
#    Callbacks (Python side)    #
#-------------------------------#
#Callbacks address which data is sent to Bokeh server

def callback_Connect_to_eLoaD():
    """Still requires modification, to verify the port"""
    global eLoaD_Connection
    global eLoaD_COM

    eLoaD.port = Port_input.value
    if eLoaD.isOpen() == False:     #If not connected, try to connect
        try:
            eLoaD.open()
        except:
            print("Port not found")
    
    eLoaD_Connection = eLoaD.isOpen()   #This flag stores the status of connectivity...
    
    #...and change GUI message
    if eLoaD_Connection is True:
        Comm_Status_Message.text = "Status: Connected"
    else:
        Comm_Status_Message.text = "Status: Port not found"

#This has priority, locked task
@gen.coroutine
@without_document_lock
@count()
def acquire_data_fake(t):
    global acquiredData
    print(t)
    x = []
    for i in range(0+5*t,4+5*t):
        x.append(i)
    new_x = np.asarray(x)
    new_y = np.sin(new_x/10)
    # Format to roll into azquiredData dictionary
    new = dict(timestamp=new_x, raw_data=new_y)
    # Roll new data into acquiredData, rollover of 1000
    rollDict(new, acquiredData, 200)
    print(t)

#This has priority, locked task
@gen.coroutine
@without_document_lock
@count()
def acquire_data_fake_2(t):
    global acquiredData
    print(t)
    x = []
    y = []
    for i in range(0+5*t, 4+5*t):
        x.append(CV['Voltage'].iloc[i])
        y.append(CV['Current'].iloc[i])
    new_x = np.asarray(x)
    new_y = np.asarray(y)
    # Format to roll into azquiredData dictionary
    new = dict(timestamp=new_x, raw_data=new_y)
    # Roll new data into acquiredData, rollover of 1000
    rollDict(new, acquiredData, 200)
    print(t)



@gen.coroutine
@count()
def acquire_data(t):
    global acquiredData
    new_x = np.random.uniform(-1, 1, size=5)
    new_y = np.random.uniform(-10, 10, size=5)
    # Format to roll into azquiredData dictionary
    new = dict(timestamp=new_x, raw_data=new_y)
    # Roll new data into acquiredData, rollover of 1000
    rollDict(new, acquiredData, 200)
    print("Acquire data")
    print(len(acquiredData['timestamp']))

#------------------------------#
#    Callbacks (Server Side)   #
#------------------------------#
#Server react to data as sent by Python, defined by JS code.
#
#Deprecated
callback_status = CustomJS(args=dict(Port=Port_input, Message=Comm_Status_Message, conStatus = eLoaD_Connection), 
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
    """This callback should start experiment"""
    global eLoaD_Connection
    global Start
    if eLoaD_Connection is False:
        print("eLoaD not connected")
        Start.label = "Start"
    else:
        print("eLoad is connected")
        Start.label = "Stop"
    

def callback_Random():
    """For random stuff"""
    global eLoaD_Connection
    
    if eLoaD_Connection is True:
        Comm_Status_Message.text = "Status: Not connected"
        eLoaD_Connection = False
    else:
        Comm_Status_Message.text = "Status: Connected"
        eLoaD_Connection = True
    print(eLoaD_Connection)

"""Callback Assignments"""
Connect.on_click(callback_Connect_to_eLoaD)
Save.js_on_click(callback_Save)
Start.on_click(callback_Start)
Random_test.on_click(callback_Random)


#Plotting has no priority, also slower
@count()
@gen.coroutine
#@without_document_lock

def update_plot(t):
    global acquiredData
    source.stream(
        {'time': acquiredData['timestamp'], 'raw_data': acquiredData['raw_data']}, 500)
    source.stream(
        {'time': acquiredData['timestamp'], 'raw_data': acquiredData['raw_data']}, 500)
    print(t)
    clearDict(acquiredData)
    print("Update plot")

#-----------#
#    GUI    #
#-----------#
"""Front End"""
Comm_Panel = row(Port_input, Comm_Status_Message)
Voltage = column(Voltage_Start, Voltage_Floor, Voltage_Ceiling)
Panel = row(column(Comm_Panel, Connect, Gain, Voltage, Start, Save),plot_raw)
#Panel = row(column(Comm_Panel, Connect, Gain, Voltage, Start,
#                   Save, Random_test), plot_raw, plot_current, Table)

curdoc().add_periodic_callback(update_plot,3000)
curdoc().add_periodic_callback(acquire_data_fake_2,200)
curdoc().add_root(Panel)


#Lists of things TODO now:
    #DONE - Generate random data, plot it
    #DONE Make sure it plots only recent data for the mean time
    #DONE Solve multithreading strategy
    #DONE Solved callback for eLoaD connectivity
    
    #RESEARCH I THINK that the reason eLoaD doesn't read correctly, is because i don't assign ALNn to
    #the  AINCOM (1.5), don't forget this is a differential readout.
    
    #TODO- Now update dictionaries to contain an extra column for current calulation
    #-Acquire data form eLoaD
        #-Tranfer Knowledge from previous work
        #-Make it somewhat interactive with interface
        #-Plot current...
        #-Start working with th eLoaD!