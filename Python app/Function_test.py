import numpy as np

x = np.linspace(0, 9, 10)

def funcion(variable):
    variable += 1
    return variable
    
for binny in range(0,10):
    print(funcion(x))
    print(x)
