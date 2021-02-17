#!/usr/bin/env python3

import mms
import sympy

rho = 'cos(x)'
rho_u = '2*sin(x)'
rho_et = '3*cos(x)'
p = 'rho'
vel = 'rho_u / rho * e_i'
rho_ht = 'rho_et + p'

f_rho, e_rho = mms.evaluate('div(vel*rho)', rho, variable='rho', rho=rho, rho_u=rho_u, vel=vel)
f_rho_u, e_rho_u = mms.evaluate('div(vel*rho_u) + grad(p).dot(e_i)', rho_u, variable='rho_u', rho=rho, rho_u=rho_u, vel=vel, p=p)
f_rho_et, e_rho_et = mms.evaluate('div(vel*rho_ht)', rho_et, variable='rho_et', rho_et=rho_et, rho=rho, rho_u=rho_u,  vel=vel, p=p, rho_ht=rho_ht)

mms.print_hit(e_rho, 'exact_rho')
mms.print_hit(f_rho, 'forcing_rho')

mms.print_hit(e_rho_u, 'exact_rho_u')
mms.print_hit(f_rho_u, 'forcing_rho_u')

mms.print_hit(e_rho_et, 'exact_rho_et')
mms.print_hit(f_rho_et, 'forcing_rho_et')
