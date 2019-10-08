def rollDict(data, object, rollover):
    """Rolls data into position 0
    Has a rollover to keep a maximum size of dictionary (therefore data is lost)"""
    for key in object.keys():
        object[key].insert(0, data[key][0])
        if len(object[key]) > rollover:
            object[key].pop()
    print(object)

def clearDict(object):
    """Clears the dictionary items, preserves the keys"""
    for key in object.keys():
        object[key].clear()
