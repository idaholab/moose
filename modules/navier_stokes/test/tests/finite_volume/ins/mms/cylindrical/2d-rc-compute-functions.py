#!/usr/bin/env python3

import mms
import sympy

vel = 'sin(x) * e_i + cos(y) * e_k'
p = 'sin(x)*cos(y)'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', 'sin(x)', variable='u', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_v, e_v = mms.evaluate('div(vel*rho*v) - div(mu * grad(v)) + grad(p).dot(e_k)', 'cos(y)', variable='v', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_p, e_p = mms.evaluate('div(vel)', p, variable='p', vel=vel, transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))

rho = sympy.Symbol('rho')

e_rhou = e_u * rho
e_rhov = e_v * rho

mms.print_hit(e_u, 'exact_u')
mms.print_hit(e_rhou, 'exact_rhou', rho='${rho}')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')

mms.print_hit(e_v, 'exact_v')
mms.print_hit(e_rhov, 'exact_rhov', rho='${rho}')
mms.print_hit(f_v, 'forcing_v', mu='${mu}', rho='${rho}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p')
