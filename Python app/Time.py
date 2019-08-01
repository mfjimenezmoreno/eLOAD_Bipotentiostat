import time

timer = [0,0]
timer[0] = time.time_ns()
timer[1] = time.time_ns()

print(str(10E-9*(timer[1] - timer[0])) + ' ns')
