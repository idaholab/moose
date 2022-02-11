#!/usr/bin/env python3

import mms
import sympy

u = 'cos(pi/2 * x)'
vel = u + '* e_i'
p = 'sin(x)'

# Select porosity model
porosity = '0.8'
porosity = '1 - 0.5 * 1 / (1 + exp(-30*(x-1)))'

# Requires smooth_porosity = true in FVKernels
f_u, e_u = mms.evaluate('div(vel*rho*u/porosity) - div(mu*porosity*grad(u/porosity)) + porosity*grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, porosity=porosity, scalars=['mu', 'rho'])
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'])

rho = sympy.Symbol('rho')

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p', rho='${rho}')
