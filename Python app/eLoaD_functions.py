#WARNING: Obsolete function
def rollDict(new_data, object, rollover):
    for key in new_data.keys():
        #Let' make sure that the code factors in more than one element insertion
        for element in new_data[key]:
            object[key].insert(0, element)
        while len(object[key]) > rollover:
            object[key].pop()
#WARNING: Obsolete function
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
