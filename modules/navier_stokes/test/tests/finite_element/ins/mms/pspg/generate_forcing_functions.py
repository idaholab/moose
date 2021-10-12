#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Note that this script requires sympy to be installed in your python environment

import sys, os

split_path = list(filter(None, os.getcwd().split('/')))
s = '/'
ns_root = ''
for sub in split_path:
    ns_root = s.join((ns_root, sub))
    if sub == 'navier_stokes':
        break
ns_python = s.join((ns_root, 'python'))
sys.path.append(ns_python)

from ins_calc_routines import *
from sympy import *
import sympy as sp
import numpy as np

x, y = var('x y')

u = 0.4*sin(0.5*pi*x) + 0.4*sin(pi*y) + 0.7*sin(0.2*pi*x*y) + 0.5
v = 0.6*sin(0.8*pi*x) + 0.3*sin(0.3*pi*y) + 0.2*sin(0.3*pi*x*y) + 0.3
p = 0.5*sin(0.5*pi*x) + 1.0*sin(0.3*pi*y) + 0.5*sin(0.2*pi*x*y) + 0.5
vxx = diff(u, x)
px = diff(p, x)

uvec = sp.Matrix([u, v])

volume_source = {
                'vel_x' : prep_moose_input(L_momentum_laplace_no_turbulence(uvec, p, x, y)[0]),
                'vel_y' : prep_moose_input(L_momentum_laplace_no_turbulence(uvec, p, x, y)[1]),
                'p' : prep_moose_input(L_pressure(uvec, x, y))}
solution_dict = {'vel_x' : u, 'vel_y' : v, 'p' : p, 'vxx' : vxx, 'px' : px}

h_list = ['4', '8', '16', '32']
h_array = np.array([.25, .125, .0625, .03125])
for key, value in solution_dict.items():
    print("The solution function for %s is %s" % (key, value))
for key, value in volume_source.items():
    print("The forcing function for %s is %s" % (key, value))
