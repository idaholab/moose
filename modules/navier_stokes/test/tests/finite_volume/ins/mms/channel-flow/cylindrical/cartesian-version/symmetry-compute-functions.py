#!/usr/bin/env python3

import mms
import sympy

u = 'sin(pi*x)*cos(pi*y)'
v = 'cos(1.3*x)*cos(pi*y)'

vel = u + '* e_i + ' + v + ' * e_j'

p = 'cos(1.5*x)*cos(1.6*y)'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'])
f_v, e_v = mms.evaluate('div(vel*rho*v) - div(mu * grad(v)) + grad(p).dot(e_j)', v, variable='v', vel=vel, p=p, scalars=['mu', 'rho'])
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'])

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')
mms.print_hit(e_v, 'exact_v')
mms.print_hit(f_v, 'forcing_v', mu='${mu}', rho='${rho}')
mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p', rho='${rho}')
