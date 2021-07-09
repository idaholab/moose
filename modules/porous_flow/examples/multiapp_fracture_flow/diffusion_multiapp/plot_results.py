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

def analytical_simple(t, x):
    return 1.0 / np.sqrt(4 * np.pi * t) * np.exp(-np.power(x, 2) / 4.0 / t)

def analytical_multi(t, x):
    # the heat transfer coefficient is 0.004
    return 0.5 * (1.0 + np.exp(-2 * 0.004 * t)) * analytical_simple(t, x)

l2errors = {}

# results with no heat transfer
l2 = []
plt.figure()
xvals = range(51)
plt.plot(xvals, analytical_simple(100.0, xvals), label='analytical')
data = np.genfromtxt(r'gold/single_var_final_csv_final_results_0001.csv', delimiter=',', names=True)
plt.plot(data['x'], data['T'], linestyle='dotted', marker='.', label='dt = 100')
l2.append(np.sqrt(sum([np.power(data['T'][i] - analytical_simple(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/single_var_final_csv_final_results_0010.csv', delimiter=',', names=True)
plt.plot(data['x'], data['T'], linestyle='dotted', marker='.', label='dt = 10')
l2.append(np.sqrt(sum([np.power(data['T'][i] - analytical_simple(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/single_var_final_csv_final_results_0100.csv', delimiter=',', names=True)
plt.plot(data['x'], data['T'], '.', label='dt = 1')
l2.append(np.sqrt(sum([np.power(data['T'][i] - analytical_simple(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
plt.grid()
plt.legend()
plt.title("Diffusion (no heat transfer) at t=100")
plt.xlim([0, 50])
plt.xlabel("x")
plt.ylabel("T")
plt.savefig("diffusion_single_var.png")
#plt.show()
plt.close()
l2errors['single_var'] = {}
l2errors['single_var']['dt'] = [100, 10, 1]
l2errors['single_var']['l2'] = l2

# results with heat transfer but not using a MultiApp
l2 = []
plt.figure()
xvals = range(51)
plt.plot(xvals, analytical_multi(100.0, xvals), label='analytical')
data = np.genfromtxt(r'gold/two_vars_final_csv_final_results_0001.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], linestyle='dotted', marker='.', label='dt = 100')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/two_vars_final_csv_final_results_0010.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], linestyle='dotted', marker='.', label='dt = 10')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/two_vars_final_csv_final_results_0100.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], '.', label='dt = 1')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
plt.grid()
plt.legend()
plt.title("Diffusion with heat transfer (no MultiApp) at t=100")
plt.xlim([0, 50])
plt.xlabel("x")
plt.ylabel("T")
plt.savefig("diffusion_two_vars.png")
#plt.show()
plt.close()
l2errors['two_vars'] = {}
l2errors['two_vars']['dt'] = [100, 10, 1]
l2errors['two_vars']['l2'] = l2

# results with MultiApp using transfer of temperature
l2 = []
plt.figure()
xvals = range(51)
plt.plot(xvals, analytical_multi(100.0, xvals), label='analytical')
data = np.genfromtxt(r'gold/fracture_app_final_csv_final_results_0001.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], linestyle='dotted', marker='.', label='dt = 100')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/fracture_app_final_csv_final_results_0010.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], linestyle='dotted', marker='.', label='dt = 10')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/fracture_app_final_csv_final_results_0100.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], '.', label='dt = 1')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
plt.grid()
plt.legend()
plt.title('Diffusion with heat transfer (via "T" MultiApp) at t=100')
plt.xlim([0, 50])
plt.xlabel("x")
plt.ylabel("fracture T")
plt.savefig("diffusion_fracture_app.png")
#plt.show()
plt.close()
l2errors['fracture_app'] = {}
l2errors['fracture_app']['dt'] = [100, 10, 1]
l2errors['fracture_app']['l2'] = l2

# results with MultiApp using transfer of heat energy
l2 = []
plt.figure()
xvals = range(51)
plt.plot(xvals, analytical_multi(100.0, xvals), label='analytical')
data = np.genfromtxt(r'gold/fracture_app_heat_final_csv_final_results_0001.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], linestyle='dotted', marker='.', label='dt = 100')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/fracture_app_heat_final_csv_final_results_0010.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], linestyle='dotted', marker='.', label='dt = 10')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
data = np.genfromtxt(r'gold/fracture_app_heat_final_csv_final_results_0100.csv', delimiter=',', names=True)
plt.plot(data['x'], data['frac_T'], '.', label='dt = 1')
l2.append(np.sqrt(sum([np.power(data['frac_T'][i] - analytical_multi(100.0, data['x'][i]), 2) for i in range(len(data['x']) - 1)])))
plt.grid()
plt.legend()
plt.title('Diffusion with heat transfer (via "heat" MultiApp) at t=100')
plt.xlim([0, 50])
plt.xlabel("x")
plt.ylabel("fracture T")
plt.savefig("diffusion_fracture_app_heat.png")
#plt.show()
plt.close()
l2errors['fracture_app_heat'] = {}
l2errors['fracture_app_heat']['dt'] = [100, 10, 1]
l2errors['fracture_app_heat']['l2'] = l2

plt.figure()
plt.loglog(l2errors['single_var']['dt'], l2errors['single_var']['l2'], label = 'No heat transfer')
plt.loglog(l2errors['two_vars']['dt'], l2errors['two_vars']['l2'], label = 'Coupled, no MultiApp')
plt.loglog(l2errors['fracture_app_heat']['dt'], l2errors['fracture_app_heat']['l2'], linestyle='dashed', label = 'Coupled, "heat" MultiApp')
plt.loglog(l2errors['fracture_app']['dt'], l2errors['fracture_app']['l2'], linestyle='dotted', label = 'Coupled, "T" MultiApp')
plt.grid()
plt.legend()
plt.xlabel("dt")
plt.ylabel("L2 error")
plt.title("L2 error in each approach")
plt.savefig("diffusion_l2_error.png")
plt.show()
plt.close()
