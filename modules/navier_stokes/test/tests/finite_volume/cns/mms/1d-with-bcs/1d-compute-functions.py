#!/usr/bin/env python3

import mms
import sympy

# The highly specific floats below were computed based on inlet conditions of vel=1,T=.01,p=10
# Those primitive values were selected such that the conserved variable values (the float coeffs
# below) were relatively close to unity while ensuring that the velocity anywhere in the domain
# is always below the speed of sound, e.g. subsonic everywhere
rho = '3.487882614709243*cos(x)'
rho_ud = '3.487882614709243*eps*cos(1.1*x)'
rho_et = '26.74394130735463*cos(1.2*x)'
ud = 'rho_ud / rho'
u = 'ud / eps'
vel = 'u * e_i'
mass_flux = 'rho_ud * e_i'
e = 'rho_et / rho - 0.5 * vel.dot(vel)'
# 0.4 = gamma - 1
p = '0.4 * e * rho'
rho_ht = 'rho_et + p'
ht = 'rho_ht / rho'
gamma = 1.4
R = 8.3145
molar_mass = 29.0e-3
R_specific = R / molar_mass
cp = gamma * R_specific / (gamma - 1.)
cv = cp / gamma
T = 'e / ' + str(cv)
eps_p = 'eps * p'

f_rho, e_rho = mms.evaluate('div(mass_flux)', rho, variable='rho', rho_ud=rho_ud, mass_flux=mass_flux, scalars=['eps'])
f_rho_ud, e_rho_ud = mms.evaluate('div(mass_flux * u) + eps*grad(p).dot(e_i)', rho_ud, variable='rho_ud', rho=rho, rho_ud=rho_ud, mass_flux=mass_flux, ud=ud, u=u, rho_et=rho_et, vel=vel, e=e, p=p, scalars=['eps'])
f_rho_et, e_rho_et = mms.evaluate('div(mass_flux * ht)', rho_et, variable='rho_et', rho_et=rho_et, rho=rho, rho_ud=rho_ud, mass_flux=mass_flux, ud=ud, u=u, vel=vel, e=e, p=p, rho_ht=rho_ht, ht=ht, scalars=['eps'])
_, e_T = mms.evaluate('T', T, variable='T', rho=rho, rho_et=rho_et, rho_ud=rho_ud, ud=ud, u=u, vel=vel, e=e, T=T, scalars=['eps'])
_, e_eps_p = mms.evaluate('eps_p', eps_p, variable='eps_p', rho=rho, rho_et=rho_et, rho_ud=rho_ud, ud=ud, u=u, vel=vel, e=e, p=p, scalars=['eps'])
_, e_p = mms.evaluate('p', p, variable='p', rho=rho, rho_et=rho_et, rho_ud=rho_ud, ud=ud, u=u, vel=vel, e=e, p=p, scalars=['eps'])

mms.print_hit(e_rho, 'exact_rho')
mms.print_hit(f_rho, 'forcing_rho', eps='${eps}')

mms.print_hit(e_rho_ud, 'exact_rho_ud', eps='${eps}')
mms.print_hit(f_rho_ud, 'forcing_rho_ud', eps='${eps}')

mms.print_hit(e_rho_et, 'exact_rho_et')
mms.print_hit(f_rho_et, 'forcing_rho_et', eps='${eps}')

mms.print_hit(e_T, 'exact_T', eps='${eps}')
mms.print_hit(e_eps_p, 'exact_eps_p', eps='${eps}')
mms.print_hit(e_p, 'exact_p', eps='${eps}')
