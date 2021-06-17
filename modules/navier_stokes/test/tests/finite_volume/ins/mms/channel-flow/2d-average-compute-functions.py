#!/usr/bin/env python3

import mms
import sympy

u = 'cos(pi * x / 2) * sin(pi * y / 2)'
v = 'sin(pi * x / 4) * cos(pi * y / 2)'
vel = u + '* e_i + ' + v + ' * e_j'

p = 'cos(pi * x / 4) * sin(3 * pi * y / 2)'

temp = 'sin(pi * x / 4) * cos(pi * y / 2)'

f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'])
f_v, e_v = mms.evaluate('div(vel*rho*v) - div(mu * grad(v)) + grad(p).dot(e_j)', v, variable='v', vel=vel, p=p, scalars=['mu', 'rho'])
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'])
f_t, e_t = mms.evaluate('div(vel*rho*cp*temp) - div(k * grad(temp))', temp, variable='temp', vel=vel, scalars=['rho', 'cp', 'k'])

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

mms.print_hit(e_t, 'exact_t')
mms.print_hit(f_t, 'forcing_t', k='${k}', rho='${rho}', cp='${cp}')
