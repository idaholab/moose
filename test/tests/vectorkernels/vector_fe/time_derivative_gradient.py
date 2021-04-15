#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from sympy import *
'''
This script is used to generate the forcing functions for
the electromagnetic_coulomb_gauge mms test
'''

def laplace(f):
    return diff(diff(f, x), x) + diff(diff(f, y), y)

def ddt2(f):
    return diff(diff(f, t), t)

def grad_ddt(f, comp):
    return diff(diff(f, t), comp)

x, y, z, t = symbols('x y z t')
phi = cos(1.1*x)*cos(1.2*y)*cos(.3*t)
Ax = cos(.4*x)*cos(.5*y)*cos(.3*t)
Ay = cos(.6*x)*cos(.7*y)*cos(.3*t)

phi_source = -5*laplace(phi) + diff(diff(phi, t), x)
Ax_source = -(5*laplace(Ax) - ddt2(Ax) - grad_ddt(phi, x))
Ay_source = -(5*laplace(Ay) - ddt2(Ay) - grad_ddt(phi, y))

print(phi)
print(Ax)
print(Ay)
print("")
print(phi_source)
print(Ax_source)
print(Ay_source)
