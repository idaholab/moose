import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import os.path

my_path = os.path.abspath(os.path.dirname(__file__))
path = os.path.join(my_path, "./porous_flow_geochem_out.csv")

MOOSE_data = pd.read_csv(path, skiprows=[1])

MOOSE_data2 = MOOSE_data.iloc[:, 2:16].copy()
MOOSE_data2.sort_values(by=1, axis=1, inplace=True)

MOOSE_label = MOOSE_data2.columns

time1 = 500  # 128
time2 = 1561
figure, axis = plt.subplots(1, 2, figsize=(18, 6))
unit_convert = 1/3600
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']

# for i in MOOSE_label:
#   plt.plot(MOOSE_data['time'][:time1]*unit_convert,
#            MOOSE_data[i][:time1], label=i)

for i in MOOSE_label:
  if (i == 'delta_HCO3' or i == 'darcy_velocity'):
    continue
  axis[0].plot(MOOSE_data['time'][:]*unit_convert,
               MOOSE_data[i][:], label=i)
axis[0].legend(loc='upper left', bbox_to_anchor=(0.05, 1))
#axis[0].set_ylim(-1e-2, 1)

for ax in axis.flat:
    ax.set(xlabel='time(days)', ylabel='kg.s$^{-1}$')

# for i in MOOSE_label:
#   axis[1].plot(MOOSE_data['time'][time1+1:time2]*unit_convert,
#                MOOSE_data[i][time1+1:time2], label=i)
#   axis[1].legend(loc='upper left', bbox_to_anchor=(1.05, 1.01))

axis[1].plot(MOOSE_data['time'][:]*unit_convert,
               MOOSE_data['delta_HCO3'][:], label='delta_HCO3')
axis[1].legend(loc='upper left', bbox_to_anchor=(1.05, 1.01))

for ax in axis.flat:
    ax.set(xlabel='time(hrs)', ylabel='mass_frac')


figure.suptitle('Species net change')

plt.legend()
plt.savefig('results/species_change.png', dpi=300)
plt.show()

plt.plot(MOOSE_data['time'][:time1]*unit_convert,MOOSE_data['avg_porosity'][:time1])
plt.ylim([0.1, 0.13])
plt.title('Porosity')
plt.xlabel('time(hrs)')
plt.savefig('results/porosity_change.png', dpi=300)
