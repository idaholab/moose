#!/usr/bin/env python

import pandas
import matplotlib.pyplot as plt
import numpy as np
import math

ax = plt.gca()

case1 = pandas.read_csv('no_fracture.csv')
strain1 = case1['time']
stress1 = case1['stress_yy']

case2 = pandas.read_csv('w0_0_l.csv')
strain2 = case2['time']
stress2 = case2['stress_yy']

case3 = pandas.read_csv('w0_10_l.csv')
strain3 = case3['time']
stress3 = case3['stress_yy']

case4 = pandas.read_csv('w0_50_l.csv')
strain4 = case4['time']
stress4 = case4['stress_yy']

case5 = pandas.read_csv('w0_100_l.csv')
strain5 = case5['time']
stress5 = case5['stress_yy']

plt.plot(strain1,stress1,label='No fracture')
plt.plot(strain2,stress2, label='W0 = 0')
plt.plot(strain3,stress3, label='W0 = 1e7')
plt.plot(strain4,stress4, label='W0 = 5e7')
plt.plot(strain5,stress5, label='W0 = 1e8')

plt.title("Stress")
plt.xlabel("Total Strain")
plt.ylabel("Stress")

ax.legend()
ax.set_xlim(left=0)
ax.set_ylim(bottom=0)
plt.savefig('Fig8.pdf')
