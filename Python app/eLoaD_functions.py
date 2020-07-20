class UnacceptedParameter(Exception):
    """Exception to handle non-legal parameters"""
    def __init__(self, data):
        self.data = data
    
    def __str__(self):
        return repr(self.data)

from importlib import import_module

import pygatt
import time

class BipotSettings(object):
    
    from binascii import hexlify
    import pygatt
    
    #Constants
    GAIN_100, GAIN_3k, GAIN_30k, GAIN_300k, GAIN_3M, GAIN_30M = '100', '3k', '30k', '300k', '3M', '30M'
    CV, CA = 'Voltammetry', 'Chronoamperometry'
    SINGLE, DUAL = 'Single Mode', 'Dual Mode'
    CATHODIC, ANODIC = 'Cathodic', 'Anodic'
    #Attributes

    
    def __init__(self):
        self.running = False
        self.gain = self.GAIN_30k
        self.v1_start, self.v1_floor, self.v1_ceiling = 0.2, 0.1, 0.3
        self.v2 = 0.2
        self.technique = self.CV
        self.sweep = self.CATHODIC
        self.mode = self.SINGLE
        self.segments = 3
        self.scan_rate = 100
        self.serial = import_module('serial')
        self.eload = self.serial.Serial()
        self.eload_connected = False    #A flag to state connectivity status
        self.eload_port = None  #The port (e.g. "COM13")
        self.eload.baudrate = 9600
        self.eload.timeout = 1
    
    def __conditions_error_handling(self):
        
        if self.v1_floor > self.v1_ceiling:
            raise UnacceptedParameter(
                'Voltage Floor should be lower than ceiling')
        
        if self.v1_start >= self.v1_ceiling and self.sweep == self.ANODIC:
            print("HOLA")
            raise UnacceptedParameter(
                'Anodic sweep will not meet bounds')
        
        if self.v1_start <= self.v1_floor and self.sweep == self.CATHODIC:
            raise UnacceptedParameter(
                'Cathodic sweep will not meet bounds')
        
        if int(self.segments) < 1:
            raise UnacceptedParameter(
                'Number of segments needs to be bigger than zero')
        
        if int(self.scan_rate) <= 0:
            raise UnacceptedParameter(
                'Scan rate should be bigger than zero')
    
    def validate_conditions(self):
        self.__conditions_error_handling()
        
    
    def return_cell_conditions(self):
        """Returns a message with cel conditions and estimated experimental time"""
        message = str()
        experiment_time = 0
        time = str()
        
        experiment_time = (self.segments-1) * 1000*(self.v1_ceiling-self.v1_floor)/self.scan_rate
        if self.sweep is self.ANODIC:
            experiment_time += 1000 * (self.v1_ceiling-self.v1_start)/self.scan_rate
        else:
            experiment_time += 1000 * (self.v1_start-self.v1_floor)/self.scan_rate
        
        if experiment_time > 300:
            time = str(round(experiment_time/60, 2)) + ' minutes'
        else:
            time = str(round(experiment_time)) + ' seconds'

        message += "<b style='color:#ff1744'>Cyclic Voltametry: </b>" + self.mode + "</br>"
        message += '<b style="color:#000000">Voltage Window: </b>' + \
            str(self.v1_floor) + " - " + str(self.v1_ceiling) + " volts</br>"
        message += "<b style='color:#000000'>Voltage Start: </b>" + \
            str(self.v1_start) + " volts</br>"
        message += "<b style='color:#000000'>Scan start: </b>" + \
            str(self.sweep) + " direction</br>"
        message += "<b style='color:#000000'>Voltage scan: </b>" + \
            str(self.scan_rate) + " mV/s</br>"
        message += "<b style='color:#000000'>Segments: </b>" + \
            str(self.segments) + "</br>"
        message += "<b style='color:#000000'>Estimated time: </b>" + time + "</br>"
        return message
    
    def serial_connect(self, port="COM13"):
        self.eload_port = port
        #try:
            #self.eload.serial.
        pass
    
    
    def transfer_settings(self):
        """GUI cell settings are sent to the eLoaD
        To know the string structure, check the attribute and constant sections"""
        
        #WARNING Maybe I should put this into the error handler?
        #Check if eLOAD is connected, then send information regarding technique
        if self.serial_connect() == False:
            raise IOError("Not connected to eLOAD")
            return
        
        message = []
        message.append("T" + self.technique)
        message.append("M" + self.mode)
        if self.mode == self.DUAL:
            message.append("V2" + str(self.v2))
        message.append("VL" + str(self.v1_floor))
        message.append("VH" + str(self.v1_ceiling))
        message.append("VS" + str(self.v1_start))
        message.append("SD" + self.sweep)
        message.append("VL" + str(self.v1_ceiling))
        message.append("SR" + str(self.scan_rate))
        message.append("SE" + str(self.segments))
        #Transfer Data
    
class BLE(object):
    """
    Handles Bluetooth LE related tasks
    """
    def __init__(self, port):
        self.port = port
        self.dongle = self.pygatt.BGAPIBackend(serial_port=self.port)
    
    def bt_connect(self):
        pass

#class BLE2(pygatt.BGAPIBackend):
#    def _init_(self):
#        super()._init_()
#    
#    def connect_eLoaD(self, serialport, MAC_address):
#        self.port = serialport
#        try:
#            super().start()
#            time.sleep(5)
#            self.device = super().connect(MAC_address)
#            time.sleep(5)
#    
#    def disconnect_eLoaD(self):
#        self.device.disconnect()
#        time.sleep(5)
#        super().stop()
#    
#    pass


#WARNING: Obsolete function
def rollDict(new_data, object, rollover):
    for key in new_data.keys():
        #Let' make sure that the code factors in more than one element insertion
        for element in new_data[key]:
            object[key].insert(0, element)
        while len(object[key]) > rollover:
            object[key].pop()
#WARNING: Perhaps sbsolete function
def clear_dict(object):
    """Clears the dictionary items, preserves the keys"""
    for key in object.keys():
        object[key].clear()

def rawHexString_to_int(data='', bits=24):
    """Converts the as-received string into a number"""
    
    hexCharacters = int(bits/4)
    #If received hex string is bigger than bit size, slice MSB
    if(data.__len__() > hexCharacters):
        data = data[-hexCharacters:]
    
    data = int(data, 16)
    if data & (1 << (bits-1)):
        data -= 1 << bits
    return data

def raw_to_Volts(raw_Data, pga, v_ref):
    """Converts integer given by ADC (24-bit) reads into the measured potential"""
    Data = rawHexString_to_int(raw_Data, bits=24)
    return 2*v_ref*Data/(pga*(2**23-1))


def raw_to_Current(raw_data, pga=2, muxGain=3000, v_ref=1.5):
    """Converts raw data into transimpedance amplifier current values (amperes).
    Returns a tuple with the current and read voltage.
    raw_data: a [string] representing a 3-byte long signed hex.
    pga: Default: 2, ADC's Programmable Gain Amplifier [1, 2, 4, 8, 16, 32, 64].
    muxGain: Default 3000. The TI amplifier gain.
    v_ref: Default: 1.5 volts. ADC and Bipotentiostat floating ground.
    """
    """
    #NOTE: while a little confusing, do not forget that this ADC is
    referred to 1.5(AINCOM). It shouldn't be needed to substract 1.5 from 
    T-I reference!
    #NOTE: ADS1255 works with 3 byte long data, however it will send a
    total of 4 bytes through serial port. Getting rid of the MSB is my
    reommendation to avoid confusions.
    
    #WARNING: I may have missed the sign of current here. Check later.
    Table of expected limits The saturation voltages of TIamp are 0 - 3 volts.
    |---------------------------------------------------------------------------------------|
    | Raw Data | ADC diff readout | TIAmp output  | Current (gain 3k) | Current (gain 300k) |
    |---------------------------------------------------------------------------------------|
    |  7FFFFF  |    -1.5 volts    |   0.0 volts   |      +0.5 mA      |        +5 uA        |
    |  000000  |    +0.0 volts    |   1.5 volts   |      +0.0 mA      |        +0 uA        |
    |  800001  |    +1.5 volts    |   3.0 volts   |      -0.5 mA      |        -5 uA        |
    |---------------------------------------------------------------------------------------|
    
    """
    volts = raw_to_Volts(raw_data, pga, v_ref)
    return volts/muxGain, volts


rawHexString_to_int('800000', bits=24)
raw_to_Volts('800000', pga=2, v_ref=1.5)
raw_to_Current('800001', pga=2, muxGain=3E3, v_ref=1.5)
raw_to_Current('7FFFFF', pga=2, muxGain=3E3, v_ref=1.5)


#-----------------------------#
#   Not so useful functions   #
#-----------------------------#
#These functions don't have practical value. Tools for debugging mostly.

def volts_to_raw(volts, pga, v_ref):
    """Convert ADC readout to HEX(24-bit)"""
    #Conversion credits to Rob from stackoverflows topic Convert hex string to int and back
    i = int((volts*pga*(2**23-1))/(2*v_ref))
    return '%06x' % ((i+2**24) % 2**24)

def current_to_raw(current, pga=2, muxGain=3000, v_ref=1.5):
    """Converts raw data into transimpedance amplifier current values (amperes)."""
    volts = current * muxGain
    return volts_to_raw(volts, pga, v_ref), volts


current_to_raw(0.0005, pga=2, muxGain=3000, v_ref=1.5)
