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
from sklearn.linear_model import LinearRegression
import matplotlib.pyplot as plt

labels = {'0': '9.2m', '1': '4.6m', '2': '2.3m', '3': '1.15m'}
colours = {'0': 'g', '1': 'k', '2': 'b', '3': 'r'}

production_starts = {} # index into data at which production starts (P_out >= 10.6)
for i in ['0', '1', '2', '3']:
    data = np.genfromtxt(r'gold/fracture_only_aperture_changing_out_' + i + '.csv', delimiter=',', names=True)
    for ind in range(1, len(data)):
        if data['P_out'][ind] >= 10.6:
            production_starts[i] = ind
            break

at_2hrs = {}
plt.figure()
for i in ['0', '1', '2', '3']:
    data = np.genfromtxt(r'gold/fracture_only_aperture_changing_out_' + i + '.csv', delimiter=',', names=True)
    plt.plot(data['time'][1:production_starts[i]+1] / 3600.0, data['TK_out'][1:production_starts[i]+1] - 273, colours[i] + ":")
    plt.plot(data['time'][production_starts[i]] / 3600.0, data['TK_out'][production_starts[i]] - 273, colours[i] + "o")
    plt.plot(data['time'][production_starts[i]:] / 3600.0, data['TK_out'][production_starts[i]:] - 273, colours[i], label=labels[i])
    for ind in range(len(data)):
        if data['time'][ind] / 3600.0 > 2.0:
            t0 = data['time'][ind - 1] / 3600.0
            t1 = data['time'][ind] / 3600.0
            T0 = data['TK_out'][ind - 1] - 273
            T1 = data['TK_out'][ind] - 273
            at_2hrs[i] = (T0 * (t1 - 2.0) + T1 * (t0 - 2.0)) / (t1 - t0)
            break
plt.grid()
plt.legend()
plt.title("Production-point temperature: no heat transfer, various mesh sizes")
plt.xlim([0, 4])
plt.xlabel("time (hours)")
plt.ylabel("T (degC)")
plt.savefig("fracture_only_aperture_changing_T_out.png")
plt.show()
plt.close()

plt.figure()
xvals = np.array([9.2, 4.6, 2.3, 1.15]).reshape((-1, 1))
yvals = np.array([at_2hrs['0'], at_2hrs['1'], at_2hrs['2'], at_2hrs['3']])
plt.plot(xvals, yvals, 'o', label="Simulation")
ord = 0.15
reg = LinearRegression().fit(np.power(xvals, ord), yvals)
x2 = np.arange(1, np.power(10, ord), 0.01).reshape((-1, 1))
print(reg.intercept_, reg.coef_)
plt.plot(np.power(x2, 1.0 / ord), reg.predict(x2), label="Fit: error = size^0.15")
plt.grid()
plt.legend()
plt.xlabel("Element size (m)")
plt.ylabel("T (degC)")
plt.title("Production temperature after 2 hours")
plt.savefig("fracture_only_aperture_changing_T_2hrs.png")
plt.show()
plt.close()

plt.figure()
for i in ['0', '1', '2', '3']:
    data = np.genfromtxt(r'gold/fracture_only_aperture_changing_out_' + i + '.csv', delimiter=',', names=True)
    plt.plot(data['time'][1:production_starts[i]+1] / 3600.0, data['P_out'][1:production_starts[i]+1], colours[i] + ":")
    plt.plot(data['time'][production_starts[i]] / 3600.0, data['P_out'][production_starts[i]], colours[i] + "o")
    plt.plot(data['time'][production_starts[i]:] / 3600.0, data['P_out'][production_starts[i]:], colours[i], label=labels[i])
plt.grid()
plt.legend()
plt.title("Production-point porepressure: no heat transfer, various mesh sizes")
plt.xlim([0, 4])
plt.xlabel("time (hours)")
plt.ylabel("P (MPa)")
plt.savefig("fracture_only_aperture_changing_P_out.png")
plt.show()
plt.close()
sys.exit(0)
