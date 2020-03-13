#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from sympy import *
from fractions import Fraction

r, theta, t, c = sympify('r, theta, t, c')

# A simple Gaussian function (all derivatives wrt theta are 0).
# This script should print the following:
#
# Steady version: f = du/dt + Biharmonic(u)
# lapu = 4*c*(c*r**2 - 1)*exp(-c*r**2)
# bihu = 16*c**2*(c**2*r**4 - 4*c*r**2 + 2)*exp(-c*r**2)
#
# Time dependent version: f = du/dt + Biharmonic(u)
u = exp(-t) * exp(-c * r**2)

ut = diff(u,t)
ur = diff(u,r)
urr = diff(ur,r)
u_theta = diff(u,theta)
u_theta_theta = diff(u_theta,theta)

# Gradient component
gradu_r=ur
gradu_t=(1/r)*u_theta
# print('gradu_r = {}'.format(gradu_r))
# print('gradu_t = {}'.format(gradu_t))

lapu = simplify(urr + ur/r + u_theta_theta/r**2)
print('lapu = {}'.format(lapu))

# The biharmonic operator in polar coodinates has a bunch of nested
# derivatives in r. Here we are ignoring the theta derivatives because
# our Gaussian does not have any theta dependence.
# https://en.wikipedia.org/wiki/Biharmonic_equation
bihu = simplify((1/r) * diff(r * diff((1/r) * diff(r*diff(u,r),r),r),r))
print('bihu = {}'.format(bihu))

# 16*c**2*(c**2*r**4 - 4*c*r**2 + 2)*exp(-c*r**2 - t) - exp(-t)*exp(-c*r**2)
f = ut + bihu
print('Time-dependent forcing function = {}'.format(f))
