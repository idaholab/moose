#!/usr/bin/env python3
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import pandas as pd

data02a=pd.read_csv('therm_step02a_out_t_sampler_0006.csv')
data03=pd.read_csv('therm_step03_out_t_sampler_0006.csv')
data03a=pd.read_csv('therm_step03a_out_t_sampler_0006.csv')

plt.figure(figsize=(6.0,4.5))
ax=plt.gca()

data02a.plot(ax=ax, x='x', y='T',label='Conduction (step02a)')
data03.plot(ax=ax, x='x', y='T',label='Conduction + Time Deriv. (step03)')
data03a.plot(ax=ax, x='x', y='T',label='Conduction + Time Deriv. + Volumetric (step03a)')

ax.set_xlabel("Position")
ax.set_ylabel("Temperature")

plt.savefig('therm_soln_compare.png', bbox_inches='tight', pad_inches=0.1)
