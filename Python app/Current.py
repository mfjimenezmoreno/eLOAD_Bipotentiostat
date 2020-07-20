from bokeh.io import curdoc
from bokeh.plotting import figure, show
from bokeh.models import CustomJS, ColumnDataSource, DataRange1d, LinearAxis, BasicTickFormatter
from bokeh.models.widgets import Select, Button, Slider, Paragraph, TextInput, TableColumn, DataTable, TableColumn, Toggle, RangeSlider, RadioButtonGroup, PreText, Div
from bokeh.layouts import gridplot, column, row
from bokeh.events import ButtonClick
from bokeh.driving import count
from bokeh.document import without_document_lock

import numpy as np
import pandas as pd
from concurrent.futures import ThreadPoolExecutor
from tornado import gen
from functools import partial
import serial
import time
from eLoaD_functions import clear_dict, BipotSettings, UnacceptedParameter

from binascii import hexlify
import pygatt
import time
#import logging
#----------------------#
#    Initialization    #
#----------------------#
"""Document"""
doc = curdoc()

"""Max threadpool executors"""
#RESEARCH How many do I require?
executor = ThreadPoolExecutor(max_workers=3)

"""Global Variables"""
#NOTE: Might require to delete some of these, as now bipot oject handles cell parameters

bipot = BipotSettings()
eLoaD_Connection = False    #Used as a global flag, act according eLoaD connection
eLoaD_COM = 'COM15'         #Stores the serial port
#eLoaD = serial.Serial()
#eLoaD.baudrate = 9600
#eLoaD.timeout = 1
BLEdongle = None    #Reserved to BLE dongle
eLoaD = None        #Reserved to open BLE port
mux_gain = 3E3      #For storing transimpedance gain as chosen by MUX
pga_value = 2.0     #The gain as specified by ADC's datasheet
v_ref = 1.5         #The reference voltage
MACADDRESS = "7c:ec:79:69:62:f8"  #HMSoft address
SERVICE = "0000ffe1-0000-1000-8000-00805f9b34fb" #Custom UUID

"""Test data"""
CV = pd.read_csv(r'Cap.csv')
CV.columns = ['Voltage', 'Current']
#source = ColumnDataSource(data=CV)

"""Widgets configuration"""
Gain = Select(title="Gain", options=['100', '3k', '30k', '300k', '3M', '30M'],
              value='30k', max_width=105)
Connect = Button(label='Connect', width=320)
Random_test = Button(label='Random', button_type='warning', width=320)
Start = Button(label='Start', button_type='success', width=320)
Voltage_Window = RangeSlider(start=-1.5, end=1.5, value=(0.1,0.3),
                             step=0.01, title="Voltage Window", bar_color='#f44336')
Voltage_Start = Slider(start=-1.5, end=1.5, value=0.2,
                       step=0.01, title='Voltage Start')
Voltage_WE2 = Slider(start=-1.5, end=1.5, value=0.2,
                       step=0.01, title='Voltage Working Electrode 2', visible=False)
Sweep_direction = RadioButtonGroup(name='Sweep Direction',
                                   labels=['Cathodic', 'Anodic'], active=0)
Voltammetry_Mode = RadioButtonGroup(name='CV Mode',
                                    labels=['Single Mode', 'Dual Mode'], active=0)
Scan_rate = TextInput(title='Scan rate (mV/s):', value='100', max_width=105)
Segments = TextInput(title='Sweep Segments:', value='3', max_width=105)
Comm_Status_Message = Paragraph(text="Status: Not connected", width=160)
Port_input = TextInput(title='Port:', value='COM15', width=160)
Save = Button(label='Save', button_type='warning', width=320)
Message_Output = Div(width=320, height = 160, text="Cyclic Voltammetry GUI",
                     background='#eceff1', css_classes=["Style.css"],
                     style={'color': '#263238', 'font-family': 'Arial', 'padding': '20px',
                                'font-weight':'300','word-break':'break-word',
                                'border': 'border: 4px solid #263238', 'border-radius': '6px',
                                'word-break': 'break-word'})
#----------------------------#
#    Figure Configuration    #
#----------------------------#
"""Dataframe structure"""
#NOTE AS of now, I will only receive raw data.
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
plot_raw.x_range = DataRange1d(0.1,0.3)
plot_raw.x_range.follow = 'end'
plot_raw.x_range.follow_interval = 100
plot_raw.x_range.range_padding = 0.1
plot_raw.xaxis.axis_label = "Voltage (Volts)"
plot_raw.xaxis.axis_label_text_font_size = '12pt'
plot_raw.xaxis.axis_label_text_font_style = "bold"
plot_raw.xaxis.axis_line_width = 2
plot_raw.xaxis.major_label_text_font_size = "10pt"
plot_raw.yaxis.axis_label = "Current (Amperes)"
plot_raw.yaxis.axis_label_text_font_size = '12pt'
plot_raw.yaxis.axis_label_text_font_style = "bold"
plot_raw.yaxis.axis_line_width = 2
plot_raw.yaxis.major_label_text_font_size = "10pt"
plot_raw.yaxis.formatter = BasicTickFormatter(precision=1, use_scientific=True)
plot_raw.grid.grid_line_dash = 'dashed'
plot_raw.title.align = 'center'
plot_raw.title.text_font = 'arial'
plot_raw.title.text_font_style = 'bold'
plot_raw.title.text_font_size = '15px'
#plot_raw.add_layout(LinearAxis(axis_label="LayoutRight"), place='right')
plot_raw.axis.major_tick_line_width = 1.5
plot_raw.axis.major_tick_out = 6
plot_raw.axis.major_tick_in = 0

#FIXME The reason why circle is plotted instead of line, is that 
#information bits are getting "delayed". Solve this.

#plot_raw.line(source=source, x='time', y='raw_data', alpha=0.2,
#              line_width=3, color='navy')
plot_raw.circle(source=source, x='time', y='raw_data', alpha=0.2, size=10,
                line_width=3, color='#FF0000')

#TODO Consider exportig this file into an HTML, which may be edditable as standalone

plot_current = figure(title='Current',
                      plot_width=500,plot_height=500)
plot_current.toolbar_location = None
plot_current.x_range = DataRange1d(0.1, 0.3)
plot_current.x_range.follow = 'end'
plot_current.x_range.follow_interval = 100
plot_current.x_range.range_padding = 0
plot_current.line(source=source, x='time', y='raw_data', alpha=0.2,
              line_width=3, color='navy')

#-------------------------------#
#    Callbacks (GUI related)    #
#-------------------------------#
#Callbacks related to buttons

def callback_Connect_to_eLoaD():
    """Connects to eLoaD. Check the Written port and attempts to connect"""
    #DONE Must verify port status, does it exist?
    
    global eLoaD_Connection
    global eLoaD_COM

    eLoaD.port = Port_input.value
    #If not connected yet, try to connect:
    if eLoaD.isOpen() == False:
        try:
            eLoaD.open()
        except:
            print("Port not found")
    
    #This flag stores the status of connectivity, used later
    #WARNING Might not be the best practice! Maybe I should check the status a certain period.
    eLoaD_Connection = eLoaD.isOpen()
    
    #...and change GUI message
    if eLoaD_Connection is True:
        Comm_Status_Message.text = "Status: Connected"
        return True
    else:
        Comm_Status_Message.text = "Status: Port not found"
        return False


def callback_Connect_eLoaD_BLE():
    """Connects to dongle and BLE"""
    #NOTE: If eLoaD BLE is not found, disconnects automatically from dongle.
    #However, it may fail to reconnect to the dongle. It is advised to
    # restart app again.
    
    global dongle
    global eLoaD
    global eLoaD_Connection
    
    
    if eLoaD_Connection == False:
        dongle = pygatt.BGAPIBackend(serial_port=Port_input.value)
        try:
            dongle.start()
            time.sleep(1)
            eLoaD = dongle.connect(MACADDRESS)
            time.sleep(1)
        except:
            update_message_output(
                '<b style="color:#ff1744">Error to perform BLE connection </b>')
            message = "Status: Not connected"
            dongle.stop()
            eLoaD_Connection = False
        else:
            update_message_output(
                '<b style="color:#03A9F4">Succesful BLE connection </b>')
            message = "Status: Not connected"
            message = "Status: Connected"
            eLoaD_Connection = True
        finally:
            Comm_Status_Message.text = message
    elif eLoaD_Connection == True:
        #Do nothing
        pass
        

#This has priority, locked task
@gen.coroutine
@count()
def acquire_data_fake_2(t):
    global acquiredData
    data = acquiredData
    index_low = 0 + 5*t
    index_high = 4 + 5*t
    print(str(t) + " " + str(index_low) + " " + str(index_high))
    print(str(4+5*t) +" "+ str(len(CV['Current'])))
    x, y = [], []
    try:
        for i in range(0+5*t, 4+5*t):
            x.append(CV['Voltage'].iloc[i])
            y.append(CV['Current'].iloc[i])
        new = dict(timestamp=x, raw_data=y)
        #We extend the new data into our acquiredData variable
        for key, value in new.items():
            acquiredData[key].extend(value)
    
    except Exception:
        #Stop this stream if our appended fake data got all information from CV
        if index_high >= len(CV['Current']):
            doc.remove_periodic_callback(callback_acquire_data_fake)
            doc.remove_periodic_callback(callback_update_plot)
            bipot.running = False
            

#TODO Adjust this code for eLoaD generated data
@gen.coroutine
@count()
def acquire_data(t):
    pass

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

        for(let i = 0; i <br nrows; i++){
            let row = [];
            for(let j = 0; j <br columns.length; j++){
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
    
    #Checking if cell conditions are correct
    try:
        bipot.validate_conditions()
    except UnacceptedParameter as error:
        print(error)
        update_message_output(
            '<b style="color:#ff1744">Cell parameter error: </b>',
            str(error)[1:-1])
    else:
        update_message_output(bipot.return_cell_conditions())
    

def callback_Random_1():
    """For random stuff: Invert Status"""
    global eLoaD_Connection
    
    if eLoaD_Connection is True:
        Comm_Status_Message.text = "Status: Not connected"
        eLoaD_Connection = False
    else:
        Comm_Status_Message.text = "Status: Connected"
        eLoaD_Connection = True
    print(eLoaD_Connection)

def callback_Random_2():
    global callback_update_plot
    global callback_acquire_data_fake
    #Clean plot
    
    if bipot.running is False:
        reset_plot()
        callback_update_plot = doc.add_periodic_callback(update_plot, 1000)
        callback_acquire_data_fake = doc.add_periodic_callback(
            acquire_data_fake_2, 100)
        bipot.running = True
        
    else:
        doc.remove_periodic_callback(callback_update_plot)
        doc.remove_periodic_callback(callback_acquire_data_fake)
        bipot.running = False


def callback_Random_3():
    update_message_output('Martin','Roberto','Braulio')

def reset_plot():
    source.data = {'time':[], 'raw_data':[]}


def update_message_output(*args):
    """Updates message output panel with information of interest as passed by args.
    Such panel will provide information as errors, cell conditions, etc."""
    message = str()
    for text in args:
        message += text
        message += '\n'
    Message_Output.text = message


def update_gain(attr, old, new):
    bipot.gain = new


def update_scan_rate(attr, old, new):
    bipot.scan_rate = int(new)


def update_segments(attr, old, new):
    bipot.segments = int(new)


def update_v1_start(attr, old, new):
    bipot.v1_start = round(float(new), 2)


def update_v2(attr, old, new):
    bipot.v2 = round(float(new), 2)


def update_v1_window(attr, old, new):
    bipot.v1_floor= round(float(new[0]), 2)
    bipot.v1_ceiling = round(float(new[1]), 2)


def update_sweep(attr, old, new):
    if new is 0:
        bipot.sweep = "Cathodic"
    elif new is 1:
        bipot.sweep = "Anodic"


def update_voltammetry_mode(attr, old, new):
    if new is 0:
        bipot.mode = bipot.SINGLE
        Voltage_WE2.visible = False
    elif new is 1:
        bipot.mode = bipot.DUAL
        Voltage_WE2.visible = True

"""Callback Assignments"""
callback_update_plot = None
callback_acquire_data_fake = None
Connect.on_click(callback_Connect_eLoaD_BLE)
Save.js_on_click(callback_Save)
Start.on_click(callback_Start)
Random_test.on_click(callback_Random_3)
Gain.on_change('value', update_gain)
Scan_rate.on_change('value', update_scan_rate)
Segments.on_change('value', update_segments)
Voltage_Start.on_change('value', update_v1_start)
Voltage_WE2.on_change('value', update_v2)
Voltage_Window.on_change('value', update_v1_window)
Sweep_direction.on_change('active', update_sweep)
Voltammetry_Mode.on_change('active', update_voltammetry_mode)

#---------------------------#
#    Callbacks (Threads)    #
#---------------------------#
#Plotting has no priority, also slower

@gen.coroutine
#It seems that we cannot make streaming unlocked
def stream():
    global acquiredData
    source.stream(
        {'time': acquiredData['timestamp'], 'raw_data': acquiredData['raw_data']})
    clear_dict(acquiredData)


@gen.coroutine
def update_plot():
    #Safely add a thread for plot update
    #NOTE: This is a safe way to update the plot, but might be removed if not necessary
    doc.add_next_tick_callback(stream)


#-----------#
#    GUI    #
#-----------#
"""Front End"""
Comm_Panel = row(Port_input, Comm_Status_Message)
GainandSegment = row(Gain, Segments, Scan_rate)
Voltage = column(GainandSegment, Voltage_Start, Voltage_WE2, Voltage_Window,
                 Sweep_direction, Message_Output )
#Panel = row(column(Comm_Panel, Connect, Gain, Voltage, Start, Save),plot_raw)
Panel = row(column(Comm_Panel, Connect, Voltammetry_Mode ,Voltage, Start,
                   Save, Random_test), plot_raw, plot_current, Table)

#callback_update_plot = doc.add_periodic_callback(update_plot, 1000)
#callback_acquire_data_fake = doc.add_periodic_callback(
#    acquire_data_fake_2, 100)

doc.add_root(Panel)


#Lists of things TODO now:
    #DONE Generate random data, plot it
    #DONE Make sure it plots only recent data for the mean time
    #DONE Solve multithreading strategy
    #DONE Solved callback for eLoaD connectivity
    #DONE Plot corruption was due to duplicate streamming
    #DONE Make plot cute
    #DONE Learned how to appropiately remove callbacks
    #DONE Error handling: check if cell parameters are right
    #DONE Estimate cell experimental time
    #DONE Cell information is printed
    
    #RESEARCH I THINK that the reason eLoaD doesn't read correctly, is because i don't assign ALNn to
    #the  AINCOM (1.5), don't forget this is a differential readout.
    
    #TODO- Now update dictionaries to contain an extra column for current calulation
    #-Acquire data form eLoaD
        #-Tranfer Knowledge from previous work
        #-Make it somewhat interactive with interface
        #-Plot current...
        #-Start working with th eLoaD!

    #NOTE:  According to documentation (data acquisition), if app requires to perform blocking computation, it is possible
    #       to perform that in a different thread.
    #       But updates to the Document must be scheduled via a next-tick callback.Callback executes as
    #       soon as possible within the next Tornado event loop. It will acquire locks to do this safely.
    #WARNING:The only safe operations to perform on document from different thread are add_next_tick_callback.
    #       Any usage that directly updates the document state from another thread by calling other document methods
    #       or by setting properties on Bokeh models, RISKS DATA AND PROTOCOL CORRUPTION.
    #WARNING: It is important to save a local copy of curdoc so all threads have access to the same document.
