#!/usr/bin/env python3

import mms
import sympy

u = 'sin(x)'
vel = u + '* e_i'
p = 'cos(x)'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))

rho = sympy.Symbol('rho')

e_rhou = e_u * rho

mms.print_hit(e_u, 'exact_u')
mms.print_hit(e_rhou, 'exact_rhou', rho='${rho}')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p', rho='${rho}')
