import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import os.path

my_path = os.path.abspath(os.path.dirname(__file__))
path = os.path.join(my_path, "./basalt_mineral_test_out.csv")

ref_data = pd.read_csv("results/GWB_data.csv", skiprows=[1, 2])
MOOSE_data = pd.read_csv(path, skiprows=[1])

ref_data2 = ref_data.copy()
ref_data2.sort_values(by=1, axis=1, inplace=True)

MOOSE_data2 = MOOSE_data.copy()
MOOSE_data2.sort_values(by=1, axis=1, inplace=True)


ref_label = ref_data2.columns.drop(['time'])
MOOSE_label = MOOSE_data2.columns.drop(['time'])

time_end = 128
figure, axis = plt.subplots(1, 2, figsize=(18, 6))
unit_convert = 1/3600/24
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'w']

for i, c in zip(ref_label[:4], colors[:4]):
  axis[0].plot(ref_data['time'][:time_end]*unit_convert, ref_data[i]
               [:time_end], c=c, label=i, linestyle='dashed')

for i, c in zip(MOOSE_label[:4], colors[:4]):
  axis[0].plot(MOOSE_data['time'][:time_end]*unit_convert,
               MOOSE_data[i][:time_end], c=c, label=i)
  axis[0].legend(loc='upper right', bbox_to_anchor=(-0.1, 1))


for i, c in zip(ref_label[5:12], colors):
  axis[1].plot(ref_data['time'][:time_end]*unit_convert, ref_data[i]
               [:time_end], c=c, label=i, linestyle='dashed')

for i, c in zip(MOOSE_label[5:12], colors):
  axis[1].plot(MOOSE_data['time'][:time_end]*unit_convert,
               MOOSE_data[i][:time_end], c=c, label=i)
  axis[1].legend(loc='upper left', bbox_to_anchor=(1.05, 1.01))

for ax in axis.flat:
    ax.set(xlabel='time(days)', ylabel='mol/kg')


figure.suptitle('Full year')

plt.savefig('results/brine_composition.png', dpi=300)
plt.show()

# error plots
plt.figure(figsize=(10, 6))
for i, j in zip(ref_label[:], MOOSE_label[:]):
  plt.plot(MOOSE_data['time'][:time_end]*unit_convert,
           abs(MOOSE_data[j][:time_end]-ref_data[i][:time_end])/(MOOSE_data[j][:time_end]+ref_data[i][:time_end]), label=i)
  plt.legend(loc='upper center', bbox_to_anchor=(1.05, 1.01))

plt.xlabel('time(days)')
plt.ylabel('Relative error')
plt.savefig('results/brine_composition_error.png', dpi=200)
plt.show()
