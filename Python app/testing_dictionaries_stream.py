import numpy as np

diccionario = dict(x=[1,2,3],y=[2,4,6])

def rollDict(data, object, rollover):
    for key in object.keys():
        #Let' make sure that the code factors in more than one element insertion
        for element in data[key]:
            object[key].insert(0,element)
        while len(object[key]) > rollover:
            object[key].pop()
    print(object)

def clearDict(object):
    for key in object.keys():
        object[key].clear()


new_x = np.random.uniform(-1, 1, size=5)
new_y = np.random.uniform(-10, 10, size=5)
new_x
rollDict({'x':new_x,'y':new_y},diccionario,10)
print(diccionario)
clearDict(diccionario)