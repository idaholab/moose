#!/usr/bin/env python
import pandas
import matplotlib.pyplot as plt
from scipy.stats import norm

FILENAME = 'latin_hypercube_out_data_0000.csv'
df = pandas.read_csv(FILENAME)

yticks = list()
prob_step = (0.999 - 0.001) / 7
for i in range(8):
    prob = prob_step*i + 0.001
    yticks.append(norm.ppf(prob, 1980, 3))

fig, ax = plt.subplots(tight_layout=True)
plt.scatter(df['sample_0'], df['sample_1'], 1, 'k', marker=".")
ax.set(xlabel='Uniform', xticks=range(2004, 2011), ylabel='Normal', yticks=yticks)
ax.grid(True, color='k', alpha=0.25, linestyle='--', linewidth=1)
fig.savefig("latin_hypercube.pdf")
