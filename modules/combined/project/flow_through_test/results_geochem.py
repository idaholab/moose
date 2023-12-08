import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import os.path

my_path = os.path.abspath(os.path.dirname(__file__))
path = os.path.join(my_path, "./porous_flow_geochem_out_react0.csv")

MOOSE_data = pd.read_csv(path)


MOOSE_data2 = MOOSE_data.iloc[:,0:8].copy()
MOOSE_data2.sort_values(by=1, axis=1, inplace=True)

MOOSE_label = MOOSE_data2.columns.drop(['time'])
print(MOOSE_label)

time_end = 36
plt.figure(figsize=(10, 6))
unit_convert = 1/3600/24
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']
style = [',', 1, 0, 2, '+', '*', 'o', 'x']


for i,j in zip(MOOSE_label, style):
  plt.plot(MOOSE_data['time'][:time_end]*unit_convert, MOOSE_data[i][:time_end], label=i, marker=j, linestyle=':')
  plt.legend(loc='best', ncol=2)

plt.title('Mineralization changes')
plt.xlabel('time(days)')
plt.ylabel('Minerals($cm^3$)')
# plt.xlim([0, 0.3])
# plt.ylim([0, 0.01])
plt.savefig('results/mineralizations.png', dpi=200)
plt.show()
