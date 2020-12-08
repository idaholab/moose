#!/usr/bin/env python3

import mms
import sympy

u = 'sqrt(2) / (4.*mu) * (1. - (y-x)**2 / 2)'
v = 'sqrt(2) / (4.*mu) * (1. - (y-x)**2 / 2)'
vel = u + '* e_i + ' + v + ' * e_j'
p = '10. - (x + y) / sqrt(2)'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'])
f_v, e_v = mms.evaluate('div(vel*rho*v) - div(mu * grad(v)) + grad(p).dot(e_j)', v, variable='v', vel=vel, p=p, scalars=['mu', 'rho'])
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho', 'mu'])

rho = sympy.Symbol('rho')

e_rhou = e_u * rho
e_rhov = e_v * rho

mms.print_hit(e_u, 'exact_u', mu='${mu}')
mms.print_hit(e_rhou, 'exact_rhou', mu='${mu}', rho='${rho}')
mms.print_hit(f_u, 'forcing_u', mu='${mu}', rho='${rho}')

mms.print_hit(e_v, 'exact_v', mu='${mu}')
mms.print_hit(e_rhov, 'exact_rhov', mu='${mu}', rho='${rho}')
mms.print_hit(f_v, 'forcing_v', mu='${mu}', rho='${rho}')

mms.print_hit(e_p, 'exact_p')
mms.print_hit(f_p, 'forcing_p', rho='${rho}', mu='${mu}')
