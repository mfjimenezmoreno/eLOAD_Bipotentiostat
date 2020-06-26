import os
import pandas as pd

#Input the SD card address here
address = 'E:'
entries = os.listdir(address)

#Enlist found files and prompt the user to choose one by index
print("Files found, type filenumber to choose")
print("Index    Filename")

for index, file in enumerate(entries, start=1):
    print(f'{index} {file}')

index = int(input()) - 1
#Be aware that you can't choose System Volume Information
data = pd.read_csv(address + entries[index], delimiter='\t')