#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import numpy as np
import matplotlib.pyplot as plt


l2errors = {}

# benchmark results (no MultiApp)
l2 = []
plt.figure()
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0801.csv', delimiter=',', names=True)
correct = data['frac_T']
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0101.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 1')
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0051.csv', delimiter=',', names=True)
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0021.csv', delimiter=',', names=True)
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0011.csv', delimiter=',', names=True)
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
plt.plot(data['x'], data['frac_T'], linestyle='dashed', label='dt = 10')
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0005.csv', delimiter=',', names=True)
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0003.csv', delimiter=',', names=True)
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/no_multiapp_frac_T_frac_T_0002.csv', delimiter=',', names=True)
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
plt.plot(data['x'], data['frac_T'], label='dt = 100')
plt.grid()
plt.legend()
plt.title("Fracture temperature at t=100, using no MultiApp")
plt.xlim([0, 10])
plt.xlabel("x")
plt.ylabel("T")
plt.savefig("frac_no_multiapp_frac_T.png")
plt.close()
l2errors['no_multiapp'] = {}
l2errors['no_multiapp']['dt'] = [2, 5, 10, 25, 50, 100]
l2errors['no_multiapp']['l2'] = l2



# conforming MultiApp
l2 = []
plt.figure()
data = np.genfromtxt(r'gold/matrix_app_dirac_out_fracture_app0_frac_T_frac_T_0100.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 1')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_dirac_out_fracture_app0_frac_T_frac_T_0020.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 5')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_dirac_out_fracture_app0_frac_T_frac_T_0010.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 10')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_dirac_out_fracture_app0_frac_T_frac_T_0004.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 25')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_dirac_out_fracture_app0_frac_T_frac_T_0002.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 50')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_dirac_out_fracture_app0_frac_T_frac_T_0001.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 100')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
plt.grid()
plt.legend()
plt.title("Fracture temperature at t=100, using MultiApp on a conforming mesh")
plt.xlim([0, 30])
plt.ylim([0, 1])
plt.xlabel("x")
plt.ylabel("T")
plt.close()
l2errors['multiapp_conforming'] = {}
l2errors['multiapp_conforming']['dt'] = [1, 5, 10, 25, 50, 100]
l2errors['multiapp_conforming']['l2'] = l2


# nonconforming MultiApp
l2 = []
plt.figure()
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0100.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 1')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0050.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 2')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0020.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 5')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0010.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 10')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0004.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 25')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0002.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 50')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/matrix_app_nonconforming_out_fracture_app0_frac_T_frac_T_0001.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], label='dt = 100')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - correct[i], 2) for i in range(len(data['x']) - 1)])))
plt.grid()
plt.legend()
plt.title("Fracture temperature at t=100, using MultiApp on a non-conforming mesh")
plt.xlim([0, 30])
plt.ylim([0, 1])
plt.xlabel("x")
plt.ylabel("T")
plt.close()
l2errors['multiapp_nonconforming'] = {}
l2errors['multiapp_nonconforming']['dt'] = [1, 2, 5, 10, 25, 50, 100]
l2errors['multiapp_nonconforming']['l2'] = l2



plt.figure()
plt.loglog(l2errors['no_multiapp']['dt'], l2errors['no_multiapp']['l2'], '.-', label = 'No MultiApp')
plt.loglog(l2errors['multiapp_conforming']['dt'], l2errors['multiapp_conforming']['l2'], '.-', label = 'MultiApp, conforming mesh')
plt.loglog(l2errors['multiapp_nonconforming']['dt'], l2errors['multiapp_nonconforming']['l2'], '.-', label = 'MultiApp, non-conforming mesh')
plt.grid()
plt.legend()
plt.xlabel("dt")
plt.ylabel("L2 error")
plt.title("Fracture temperature L2 error in each approach")
plt.savefig("frac_l2_error.png")
plt.show()
plt.close()
