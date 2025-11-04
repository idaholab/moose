#!/usr/bin/env python3

import mms
import sympy

u = 'cos(pi/2 * x) * sin(y)'
v = 'sin(x) * cos(pi/2 * y)'
vel = u + '* e_i +' + v + ' * e_j'
p = 'sin(x)*sin(y)'

f_u, e_u = mms.evaluate('div(rho*vel*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'])
f_v, e_v = mms.evaluate('div(rho*vel*v) - div(mu * grad(v)) + grad(p).dot(e_j)', v, variable='v', vel=vel, p=p, scalars=['mu', 'rho'])
f_p, e_p = mms.evaluate('div(vel)', p, variable='p', vel=vel)

mms.print_hit(e_u, 'exact_u')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')

mms.print_hit(e_v, 'exact_v')
mms.print_hit(f_v, 'forcing_v', mu='${mu}', rho='${rho}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p')
