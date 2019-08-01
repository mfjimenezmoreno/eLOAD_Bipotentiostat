import numpy as np
import matplotlib
import matplotlib.pyplot as plt

A = np.arange(-8, 8, 1)
B = [None] * np.size(A)
B2 = [None] * len(A)

for x in A:
    B[x] = str(hex(A[x]))
    B2[x] = str(hex((2**8 + A[x]) & 0x0FF))         #This lets me create the two complement data in string, also mask that extra bit when it is positive number
print(B)
print(B2)
C = np.empty(np.size(B))
C2 = np.empty(len(B2))
C3 = np.empty(len(B2))
int('FF', 16)-2**8

for x in range(len(B)):
    C[x] = int(B[x], 16)
    C2[x] = int(B[x]), 16)
    C3[x]= int(B[x]), 16) - 2**8
print(B)
print(C)

print(B2)
print(C2)
print(C3)


 # Data for plotting

fig, ax=plt.subplots()
ax.plot(C3)

ax.set(xlabel='time (s)', ylabel='voltage (mV)', title='About as simple as it gets, folks')
ax.grid()

fig.savefig("test.png")
plt.show()
