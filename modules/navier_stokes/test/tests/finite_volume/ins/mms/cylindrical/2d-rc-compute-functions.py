#!/usr/bin/env python3

import mms
import sympy

u = 'sin(pi * x) * sin(y)'
v = 'sin(x) * cos(pi/2 * y)'
vel = u + '* e_i +' + v + ' * e_k'
p = 'sin(y)'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_v, e_v = mms.evaluate('div(vel*rho*v) - div(mu * grad(v)) + grad(p).dot(e_k)', v, variable='v', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))

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
mms.print_hit(f_p, 'forcing_p', rho='${rho}')
