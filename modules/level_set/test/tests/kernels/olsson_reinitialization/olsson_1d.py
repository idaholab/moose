#!/usr/bin/env python
import matplotlib.pyplot as plt
import pandas
import glob

tdata = pandas.read_csv('output/olsson_1d_out_line_time.csv')
ax = plt.subplot(111)


for index, row in tdata.iterrows():
    filename = 'output/olsson_1d_out_line_{:04d}.csv'.format(int(row['timestep']))
    data = pandas.read_csv(filename)
    ax.plot(data['x'], data['phi'])

plt.show()
