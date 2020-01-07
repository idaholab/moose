#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This file is used to compute the gold values for the SSP Runge-Kutta test
# input file explict_ssp_runge_kutta.i.

import numpy as np
from math import exp, log

class SSPRK(object):
  def __init__(self, name, a, b, c):
    self.name = name
    self.a = a
    self.b = b
    self.c = c
    self.s = len(b)

  def solve(self, u0, f, dt, t_final):
    t = 0
    u = u0
    while t < t_final:
      dt_step = min(dt, t_final - t)
      u = self.solveStep(u, f, dt_step, t)
      t += dt_step

    return u

  def solveStep(self, u_old, f, dt, t_old):
    u = [0.0] * (self.s + 1)
    u[0] = u_old

    for i in range(self.s):
      for k in range(i + 1):
        u[i + 1] += self.a[i][k] * u[k]

      u[i + 1] += self.b[i] * dt * f(u[i], t_old + self.c[i] * dt)

    return u[self.s]

u0 = 1.0
dt = 0.1
t_final = 0.5

def computeExactSolution(t):
  return exp(-t) + t**3
def computeSSResidual(u, t):
  return -u + t**3 + 3*t**2

ssprk11 = SSPRK("SSPRK11", np.array([[1.0]]), [1.0], [0.0])
ssprk22 = SSPRK("SSPRK22", np.array([[1.0, 0.0], [0.5, 0.5]]), [1.0, 0.5], [0.0, 1.0])
ssprk33 = SSPRK("SSPRK33", np.array([[1.0, 0.0, 0.0], [0.75, 0.25, 0.0], [1.0/3.0, 0.0, 2.0/3.0]]), [1.0, 0.25, 2.0/3.0], [0.0, 1.0, 0.5])

for time_integrator in [ssprk11, ssprk22, ssprk33]:
  print("%s: %.15f" % (time_integrator.name, time_integrator.solve(u0, computeSSResidual, dt, t_final)))
