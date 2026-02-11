# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

# Note that this script requires sympy to be installed in your python environment

import sys, os
import importlib.util

if importlib.util.find_spec("moose_navier_stokes") is None:
    _ns_python_path = os.path.abspath(
        os.path.join(os.path.dirname(__file__), *([".."] * 6), "python")
    )
    print(_ns_python_path)
    sys.path.append(_ns_python_path)

from moose_navier_stokes.ins_calc_routines import *
from sympy import *
import sympy as sp
import numpy as np

x, y = var("x y")

u = 0.4 * sin(0.5 * pi * x) + 0.4 * sin(pi * y) + 0.7 * sin(0.2 * pi * x * y) + 0.5
ux = diff(u, x)

volume_source = {"u": prep_moose_input(L_advection(u, x, y))}
solution_dict = {"u": u, "ux": ux}

for key, value in solution_dict.items():
    print("The solution function for %s is %s" % (key, value))
for key, value in volume_source.items():
    print("The forcing function for %s is %s" % (key, value))
