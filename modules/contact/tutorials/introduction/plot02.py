#!/usr/bin/env python3

import matplotlib.pyplot as plt
import pandas as pd

#
# get the distance along a curve given by x and y coordinates
#
def getDistances(x, y):
    l = len(x)
    assert l == len(y)
    dist = [0]
    s = 0
    for i in range(1, l):
        s += ((x[i]-x[i-1])**2+(y[i]-y[i-1])**2)**0.5
        dist.append(s)
    return dist

fig = plt.figure(figsize=(12,9))
ax = fig.gca()

# start at step 0 and plot as long as we find CSV files
step = 0
while True:
    try:
      df = pd.read_csv("step02_out_normal0_%04d.csv" % step)
    except FileNotFoundError as e:
      break
    ax.plot(getDistances(df['x'], df['y']), df['pillars_normal_lm'], label = 'Step %d' % step)
    step += 1

# setup  gegend and axis labels
ax.legend(loc='upper left')
ax.set_xlabel('Distance along contact surface')
ax.set_ylabel('Contact pressure')

# write PDF figure
plt.savefig('pillars_normal.pdf', bbox_inches='tight')
