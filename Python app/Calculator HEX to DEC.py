def Digit2Hex( number , size ):
    if ( size == 1):
        return '%01x' % number
    if (size == 2):
        return '%02x' % number
    if (size == 3):
        return '%03x' % number
    if (size == 4):
        return '%04x' % number
    if (size == 5):
        return '%05x' % number
    if (size == 6):
        return '%06x' % number
    if (size == 7):
        return '%07x' % number
    if (size == 8):
        return '%08x' % number

Value = -127
print(Digit2Hex(Value, 2))
print(Digit2Hex(Value, 4))
print(Digit2Hex(Value, 6))
print(Digit2Hex(Value, 8))

