import os
import csv
from collections import deque

import matplotlib
import numpy as np
import matplotlib.pyplot as plt

schemes = ['implicit-euler', 'bdf2', 'crank-nicolson', 'dirk', 'explicit-euler', 'rk-2']

scheme_errors = {}

# Generate list of dts
dt = 1.0
dts = []
for i in range(0,10):
  dts.append(dt)
  dt = dt / 2.0

for scheme in schemes:

  errors = []

  for dt in dts:
    command = '../../../moose_test-opt -i high_order_time_integration.i Executioner/dt=' + str(dt) + ' Executioner/scheme=' + scheme
    os.system(command)

    with open('high_order_time_integration_out.csv', 'r') as csvfile:
      csv_data = csv.reader(csvfile, delimiter=',')

      # Get the last row second column
      error = deque(csv_data, 2)[0][1]
      errors.append(error)

  scheme_errors[scheme] = errors

for scheme, errors in scheme_errors.iteritems():
  plt.plot(dts, errors, label=scheme)

plt.xscale('log')
plt.yscale('log')
plt.title('Time Convergence Study')
plt.xlabel('dt (s)')
plt.ylabel('L2 Error')
plt.legend(loc='upper left')
plt.show()