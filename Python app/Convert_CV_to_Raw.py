
import pandas as pd
from eLoaD_functions import current_to_raw, raw_to_Current

CV = pd.read_csv(r'Cap.csv')
CV.columns = ['Voltage', 'Current']
CV['Raw']=CV['Current'].apply(lambda x:current_to_raw(x,muxGain=300000)[0])
CV['ADC_Volts']=CV['Current'].apply(lambda x:current_to_raw(x,muxGain=300000)[1])
CV.to_csv("Cap_raw.csv", columns=['Voltage','Raw'], index=0)

#Let's now try if our libraries werk
CV2 = pd.read_csv(r'Cap_raw.csv')
CV2['Current'] = CV2['Raw'].apply(lambda x: raw_to_Current(x, muxGain=300000)[0])
CV2.to_csv("Cap_raw_current2.csv", columns=['Voltage', 'Current'], index=0)

