def rollDict(data, object, rollover):
    for key in object.keys():
        #Let' make sure that the code factors in more than one element insertion
        for element in data[key]:
            object[key].insert(0, element)
        while len(object[key]) > rollover:
            object[key].pop()

def clearDict(object):
    """Clears the dictionary items, preserves the keys"""
    for key in object.keys():
        object[key].clear()

def rawHexSring_to_int(data, bits):
    """Converts the as-received string into a number"""
    for member in data:
        member = int(member, 16)
    #The following proceedure, ensures
        if member & (1 << (bits-1)):
            member -= 1 << bits
    return data

def raw_to_Volts(raw_Data, pga, v_ref):
    """Converts integer to what ADC reads at its input in volts"""
    Data = rawHexSring_to_int(raw_Data, 32)
    return 2*v_ref*Data/(pga*(2**24-1))

def raw_to_current(raw_data, pga, muxGain, v_ref):
    """Converts raw data into transimpedance amplifier current values (amperes)
    Returns a tuple with the current and read voltage"""
    volts = raw_to_Volts(raw_data, pga, v_ref)
    return (v_ref-volts)/muxGain, volts

