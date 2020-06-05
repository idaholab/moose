#!/usr/bin/env python3
import pandas
import matplotlib.pyplot as plt

df0 = pandas.read_csv('polynomial_out_data_0000.csv')
df1 = pandas.read_csv('polynomial_out_data_0001.csv')

fig, ax = plt.subplots(tight_layout=True)
ax.plot(df0['sample_0'], df0['sample_1'], 'bo', label="Data 1")
ax.plot(df1['sample_0'], df1['sample_1'], 'ro', label="Data 2")
ax.legend()
ax.set(xlabel='x', ylabel='y')
fig.savefig('replicated.pdf')

x0 = []; y0 = []
for i in range(3):
    df0 = pandas.read_csv('polynomial_out_data_0000.csv.{}'.format(i))
    x0 += df0['sample_0'].to_list(); y0 += df0['sample_1'].to_list()
    #df1.append(pandas.read_csv('polynomial_out_data_0001.csv.{}'.format(i)), ignore_index=True)

#df0 = pandas.DataFrame(x0, y0, columns=['sample_0', 'sample_1']) # could not get DataFrame.append to work



fig, ax = plt.subplots(tight_layout=True)
ax.plot(x0, y0, 'bo', label="Data 1")
#ax.plot(df0['sample_0'], df0['sample_1'], 'bo', label="Data 1")
#ax.plot(df1['sample_0'], df1['sample_1'], 'ro', label="Data 2")
ax.legend()
ax.set(xlabel='x', ylabel='y')
fig.savefig('distributed.pdf')
