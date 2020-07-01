class UnacceptedParameter(Exception):
    """Exception to handle non-legal parameters"""
    def __init__(self, data):
        self.data = data
    
    def __str__(self):
        return repr(self.data)

class BipotSettings:
    #Constants
    GAIN_100, GAIN_3k, GAIN_30k, GAIN_300k, GAIN_3M, GAIN_30M = '100', '3k', '30k', '300k', '3M', '30M'
    CV, CA = 'Voltammetry', 'Chronoamperometry'
    SINGLE, DUAL = 'Single Mode', 'Dual Mode'
    CATHODIC, ANODIC = 'cathodic', 'anodic'
    #Attributes
    gain = GAIN_30k
    v1_start, v1_floor, v1_ceiling = 0.2, 0.1, 0.3
    v2_start = 0.2
    technique = CV
    sweep = CATHODIC
    mode = SINGLE
    segments = 3
    
    def __init__(self):
        pass
    
    def __conditions_error_handling(self):

        if self.v1_floor > self.v1_ceiling:
            raise UnacceptedParameter('Voltage Floor should be lower than ceiling')
        
        if self.v1_start >= self.v1_ceiling and self.sweep is self.ANODIC:
            raise UnacceptedParameter('Anodic sweep will not meet bounds')
        
        if BipotSettings.v1_start <= BipotSettings.v1_floor and sweep is self.CATHODIC:
            raise UnacceptedParameter('Cathodic sweep will not meet bounds')
        
        if int(self.segments) < 1:
            raise UnacceptedParameter('Number of segments needs to be bigger than zero')
        
    
    def validate_conditions(self):
        self.__conditions_error_handling()
pass

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
