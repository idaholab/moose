#!/usr/bin/env python3
import pandas
import numpy as np
import matplotlib.pyplot as plt

df0 = pandas.read_csv('gold/latin_hypercube_out_data_0000.csv')
df1 = pandas.read_csv('gold/latin_hypercube_out_data_0001.csv')
num_samples = df1.shape[0]

bins = np.linspace(0, 1, num_samples + 1)
prob_step = 1 / num_samples
for i in range(len(bins)-1):
    lower = bins[i]
    upper = bins[i + 1]
    prob = (upper - lower) + lower

fig, ax = plt.subplots(tight_layout=True)
plt.scatter(df0['sample_0'], df0['sample_1'], 20, 'b', marker="s")
plt.scatter(df1['sample_0'], df1['sample_1'], 20, 'g', marker="o")
ax.grid(True, color='k', alpha=0.25, linestyle='--', linewidth=1)
ax.set(xticks=range(100, 210, 10), yticks=range(10, 21, 1))
fig.savefig("latin_hypercube.pdf")
