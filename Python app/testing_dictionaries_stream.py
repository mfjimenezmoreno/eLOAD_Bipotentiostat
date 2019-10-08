diccionario = dict(x=[1,2,3],y=[2,4,6])

def rollDict(data, object, rollover):
    for key in object.keys():
        object[key].insert(0,data[key][0])
        if len(object[key]) > rollover:
            object[key].pop()
    print(object)

def clearDict(object):
    for key in object.keys():
        object[key].clear()

clearDict(diccionario)
diccionario
