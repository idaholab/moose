#!/usr/bin/env python3

import mms
import sympy

# Zero value: left bottom right
# Zero gradient: top, left
u = 'sin(pi * y / 2) * sin(x * pi) * sin(x * pi)'

# Prescribed value: bottom
# Zero gradient: left right top
v = 'cos(y * pi) * cos(x * pi)'

vel = u + '* e_i + ' + v + ' * e_k'

# Prescribed value: outlet
# Zero gradient: left right bottom
p = 'cos(y * pi / 2) * cos(x * pi)'

# Zero value: left bottom right
# Zero gradient: top
temp = 'sin(pi * y / 2) * sin(x * pi)'


f_u, e_u = mms.evaluate('div(vel*rho*u) - div(mu * grad(u)) + grad(p).dot(e_i)', u, variable='u', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_v, e_v = mms.evaluate('div(vel*rho*v) - div(mu * grad(v)) + grad(p).dot(e_k)', v, variable='v', vel=vel, p=p, scalars=['mu', 'rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_p, e_p = mms.evaluate('div(vel*rho)', p, variable='p', vel=vel, scalars=['rho'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))
f_t, e_t = mms.evaluate('div(vel*rho*cp*temp) - div(k * grad(temp))', temp, variable='temp', vel=vel, scalars=['rho', 'cp', 'k'], transformation='cylindrical', coordinate_names=('x', 'phi', 'y'))

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
