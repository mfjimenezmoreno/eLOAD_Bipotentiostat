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
