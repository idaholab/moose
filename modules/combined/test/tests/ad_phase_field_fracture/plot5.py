#!/usr/bin/env python

import pandas
import matplotlib.pyplot as plt
import numpy as np
import math

ax = plt.gca()

case1 = pandas.read_csv('no_fracture.csv')
strain1 = case1['time']
stress1 = case1['stress_yy']

case2 = pandas.read_csv('beta_p_1.csv')
strain2 = case2['time']
stress2 = case2['stress_yy']

case3 = pandas.read_csv('beta_p_0.csv')
strain3 = case3['time']
stress3 = case3['stress_yy']

plt.plot(strain1,stress1,label='No fracture')
plt.plot(strain2,stress2, label='beta_p = 1.0')
plt.plot(strain3,stress3, label='beta_p = 0.0')

plt.title("Stress")
plt.xlabel("Total Strain")
plt.ylabel("Stress")

ax.legend()
ax.set_xlim(left=0)
ax.set_ylim(bottom=0)
plt.savefig('Fig5a.pdf')

# case1 = pandas.read_csv('no_fracture.csv')
# strain1 = case1['time']
# elastic_strain1 = case1['elastic_strain_yy']
#
# case2 = pandas.read_csv('beta_p_1.csv')
# strain2 = case2['time']
# elastic_strain2 = case2['elastic_strain_yy']
#
# case3 = pandas.read_csv('beta_p_0.csv')
# strain3 = case3['time']
# elastic_strain3 = case3['elastic_strain_yy']
#
# plt.plot(strain1,elastic_strain1,label='No fracture')
# plt.plot(strain2,elastic_strain2, label='beta_p = 1.0')
# plt.plot(strain3,elastic_strain3, label='beta_p = 0.0')
#
# plt.title("Elastic strain")
# plt.xlabel("Total Strain")
# plt.ylabel("Elastic Strain")
#
# ax.legend()
# ax.set_xlim(left=0)
# ax.set_ylim(bottom=0)
# plt.savefig('Fig5b.pdf')

# case1 = pandas.read_csv('no_fracture.csv')
# strain1 = case1['time']
# plastic_strain1 = case1['plastic_strain_yy']
#
# case2 = pandas.read_csv('beta_p_1.csv')
# strain2 = case2['time']
# plastic_strain2 = case2['plastic_strain_yy']
#
# case3 = pandas.read_csv('beta_p_0.csv')
# strain3 = case3['time']
# plastic_strain3 = case3['plastic_strain_yy']
#
# plt.plot(strain1,plastic_strain1,label='No fracture')
# plt.plot(strain2,plastic_strain2, label='beta_p = 1.0')
# plt.plot(strain3,plastic_strain3, label='beta_p = 0.0')
#
# plt.title("Plastic strain")
# plt.xlabel("Total Strain")
# plt.ylabel("Plastic Strain")
#
# ax.legend()
# ax.set_xlim(left=0)
# ax.set_ylim(bottom=0)
# plt.savefig('Fig5c.pdf')
