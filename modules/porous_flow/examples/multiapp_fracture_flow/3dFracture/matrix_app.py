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

production_starts = {} # index into data at which production starts (P_out >= 10.6)
for i in ['00', '10', '11', '21', '22']:
    data = np.genfromtxt(r'gold/matrix_app_out_fracture_app0_' + i + '.csv', delimiter=',', names=True)
    for ind in range(1, len(data)):
        if data['P_out'][ind] >= 10.6:
            production_starts[i] = ind
            break

labels = {'00': '20m, 9.2m', '10': '10m, 9.2m', '11': '10m, 4.6m', '21': '5m, 4.6m', '22': '5m, 2.3m'}
colours = {'00': 'b', '10': 'g', '11': 'r', '21': 'c', '22': 'm'}

plt.figure()
for i in ['00', '10', '11', '21', '22']:
    data = np.genfromtxt(r'gold/matrix_app_out_fracture_app0_' + i + '.csv', delimiter=',', names=True)
    plt.plot(data['time'][1:production_starts[i]+1] / 3600.0, data['TK_out'][1:production_starts[i]+1] - 273, colours[i] + ":")
    plt.plot(data['time'][production_starts[i]] / 3600.0, data['TK_out'][production_starts[i]] - 273, colours[i] + "o")
    plt.plot(data['time'][production_starts[i]:] / 3600.0, data['TK_out'][production_starts[i]:] - 273, colours[i], label=labels[i])
plt.grid()
plt.legend()
plt.title("Production temperature: with heat transfer, various mesh sizes")
plt.xlim([0, 4])
plt.ylim([160, 210])
plt.xlabel("time (hours)")
plt.ylabel("T (degC)")
plt.savefig("matrix_app_T_short.png")
plt.show()
plt.close()

plt.figure()
for i in ['00', '10', '11', '21', '22']:
    data = np.genfromtxt(r'gold/matrix_app_out_fracture_app0_' + i + '.csv', delimiter=',', names=True)
    plt.plot(data['time'][1:] / 3600.0 / 24.0, data['TK_out'][1:] - 273, colours[i], label=labels[i])
plt.grid()
plt.legend()
plt.title("Production temperature: with heat transfer, various mesh sizes")
plt.xlim([0, 1000])
plt.xlabel("time (days)")
plt.ylabel("T (degC)")
plt.savefig("matrix_app_T.png")
plt.show()
plt.close()

sys.exit(0)
