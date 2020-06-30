import pandas as pd
from eLoaD_functions import current_to_raw

CV = pd.read_csv(r'Cap.csv')
CV.columns = ['Voltage', 'Current']
CV['Raw']=CV['Current'].apply(lambda x:current_to_raw(x,muxGain=300000)[0])
CV['ADC_Volts']=CV['Current'].apply(lambda x:current_to_raw(x,muxGain=300000)[1])
CV.to_csv("Cap_raw.csv", columns=['Voltage','Raw'])