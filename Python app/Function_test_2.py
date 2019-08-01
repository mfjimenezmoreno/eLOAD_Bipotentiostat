x = 0

for x in range(0, 10):
    if x == 5:
        break               #Breaks the loop
    print(x)

print("Break at 5")

x = 0

for x in range(0, 10):
    if x ==5:
        continue            #Bypasses the rest of loop
    print(x)

print("Bypassed at 5")

x = 0

for x in range(0, 10):
    if x == 5:
        pass                # Continues normally, a formality for coding, remark, loop still suceptible to continue or break statement
    print(x)

print("Normal flow with pass statment")

