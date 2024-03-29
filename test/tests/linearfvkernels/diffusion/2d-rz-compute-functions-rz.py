#!/usr/bin/env python3

import mms
import sympy

u = '(1.5-x*x)*(1.5-y*y)'
mu = '1+0.5*x*y'

f_u, e_u = mms.evaluate('-div(mu * grad(u))', u, variable='u', mu=mu, transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))

mms.print_hit(e_u, 'analytic_solution')
mms.print_hit(f_u, 'source_func')
